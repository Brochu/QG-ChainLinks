// Fill out your copyright notice in the Description page of Project Settings.


#include "AlibiTxtBasedCheats.h"
#include "CaseSubsystem.h"
#include "ChainSubsystem.h"
#include "MatrixSubsystem.h"

#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogTxtCheats);

void UAlibiTxtBasedCheats::AddedToCheatManager_Implementation() {
	APlayerController *pc = GetPlayerController();
	if (pc) {
		_pc = pc;
	}

	UGameInstance* inst = UGameplayStatics::GetGameInstance(GetWorld());
	_case_system = inst->GetSubsystem<UCaseSubsystem>();
	_chain_system = inst->GetSubsystem<UChainSubsystem>();
}

void UAlibiTxtBasedCheats::RemovedFromCheatManager_Implementation() {
	_pc = nullptr;

	_case_system = nullptr;
	_chain_system = nullptr;
}

void UAlibiTxtBasedCheats::ShowCaseMeta() {
	const FCaseMetadata &meta = _case_system->file.meta;

	FString output = TEXT("Case Metadata");
	output.Appendf(TEXT("\n\t[%s] %s; %s on %s"), *meta.case_id.ToString(), *meta.title.ToString(), *meta.setting.region.ToString(), *meta.setting.date.ToString());
	output.Appendf(TEXT("\n\t%d days; %d blocks each"), meta.deadline_days, meta.blocks_per_day);
	output.Appendf(TEXT("\n\tStart text: %s::%s"), *meta.ink_file.ToString(), *meta.briefing_knot.ToString());
	output.Appendf(TEXT("\n\tStart facts: %s"), *FString::JoinBy(meta.starting_facts, TEXT(", "), [](const FName &f) { return f.ToString(); }));
	output.Appendf(TEXT("\n\tStart locations: %s"), *FString::JoinBy(meta.starting_locations, TEXT(", "), [](const FName &l) { return l.ToString(); }));
	output.Appendf(TEXT("\n\tLab queue size: %d"), meta.lab_queue_capacity);
	MessageConsole(output);
}

void UAlibiTxtBasedCheats::CaseStatus() {
	const FCaseMetadata &meta = _case_system->file.meta;
	const FCaseSaveState &save = _case_system->save;
	const int32 total_blocks   = meta.deadline_days * meta.blocks_per_day;
	const int32 blocks_per_day = FMath::Max(1, meta.blocks_per_day); // guard /0 before a case is loaded
	const int32 day            = save.used_blocks / blocks_per_day;
	const int32 block_of_day   = save.used_blocks % blocks_per_day;

	FString output = TEXT("Case Status");
	output.Appendf(TEXT("\n\tClock: block %d/%d (day %d, block-of-day %d), %d remaining"),
		save.used_blocks, total_blocks, day, block_of_day, total_blocks - save.used_blocks);
	output.Appendf(TEXT("\n\tActive location: %s"), *save.active_locid.ToString());

	output.Appendf(TEXT("\n\tKnown facts (%d): %s"),
		save.known_facts.Num(),
		*FString::JoinBy(save.known_facts, TEXT(", "), [](const FName &f) { return f.ToString(); }));

	output.Appendf(TEXT("\n\tKnown entities (%d): %s"),
		save.known_tags.Num(),
		*FString::JoinBy(save.known_tags, TEXT(", "), [](const FName &t) { return t.ToString(); }));

	output.Appendf(TEXT("\n\tLab queue (%d/%d):"), save.active_lab_requests.Num(), meta.lab_queue_capacity);
	for (const FPendingRequest &req : save.active_lab_requests) {

		const FCaseAction *act = _case_system->file.actions.FindByPredicate(
			[&req](const FCaseAction &a) { return a.action_id == req.action_id; });

		const int32 elapsed = save.used_blocks - req.block_started;
		output.Appendf(TEXT("\n\t\t- %s (started block %d, %d/%d blocks)"), *req.action_id.ToString(), req.block_started, elapsed, act ? act->delay : -1);
	}

	MessageConsole(output);
}

void UAlibiTxtBasedCheats::ListLocations() {
	MessageConsole(FString::Format(TEXT("All available locations in the case: {0}"), { *_case_system->file.meta.title.ToString()}));

	TArray<int32> location_idx = _case_system->list_location_idx();
	for (int32 i = 0; i < location_idx.Num(); i++) {
		FCaseLocation &loc = _case_system->file.locations[location_idx[i]];

		MessageConsole(FString::Format(TEXT(" - #{0} [{1}] {2}"), { i, *loc.location_id.ToString(), *loc.name.ToString() }));
	}
}

void UAlibiTxtBasedCheats::ExploreLocation() {
	const FCaseLocation &loc = _case_system->get_active_location();
	MessageConsole(FString::Format(TEXT("All available actions at {0}"), { *loc.name.ToString() }));

	TArray<int32> action_idx = _case_system->list_action_idx(loc.location_id);
	for (int32 i = 0; i < action_idx.Num(); i++) {
		FCaseAction &act = _case_system->file.actions[action_idx[i]];

		if (_case_system->action_visibility(act) == EActionVisibility::Locked) {
			// Shown but unselectable; the hint is the authored tease (design §5.3).
			MessageConsole(FString::Format(TEXT(" - #{0} [{1}][{2}] LOCKED{3}"), {
				i,
				*act.action_id.ToString(),
				*UEnum::GetValueAsString(act.verb),
				act.locked_hint.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" — %s"), *act.locked_hint.ToString())
			}));
			continue;
		}

		MessageConsole(FString::Format(TEXT(" - #{0} [{1}][{2}] {3} / cost: {4}"), {
			i,
			*act.action_id.ToString(),
			*UEnum::GetValueAsString(act.verb),
			*act.label.ToString(),
			act.cost
		}));
	}
}

void UAlibiTxtBasedCheats::MoveToLocation(int32 new_location_idx) {
	// Index-based because it's easier to type on cli. Will be ID-based in actual game with real UX
	TArray<int32> location_idx = _case_system->list_location_idx();

	if (new_location_idx < 0 || new_location_idx >= location_idx.Num()) {
		MessageConsole(FString::Format(TEXT("Invalid move: no location at index {0} ({1} available)."), { new_location_idx, location_idx.Num() }));
		return;
	}

	FName target_loc_id = _case_system->file.locations[location_idx[new_location_idx]].location_id;
	if (!_case_system->move_location(target_loc_id)) {
		MessageConsole(FString::Format(TEXT("Invalid move: could not move to '{0}'."), { *target_loc_id.ToString() }));
	}
}

void UAlibiTxtBasedCheats::ChooseAction(int32 action_idx) {
	// Index-based because it's easier to type on cli. Will be ID-based in actual game with real UX
	const FCaseLocation &loc = _case_system->get_active_location();
	TArray<int32> actions_at_location = _case_system->list_action_idx(loc.location_id);

	if (action_idx < 0 || action_idx >= actions_at_location.Num()) {
		MessageConsole(FString::Format(TEXT("Could not choose action at index {0}; action count at location: {1}"), { action_idx, actions_at_location.Num()}));
		return;
	}

	FCaseAction &act = _case_system->file.actions[actions_at_location[action_idx]];
	MessageConsole(FString::Format(TEXT("Picked action: [{0}][{1}] {2} / cost: {3}"), {
		*act.action_id.ToString(),
		*UEnum::GetValueAsString(act.verb),
		*act.label.ToString(),
		act.cost
	}));
	const ECommitActionResult res = _case_system->commit_action(act.action_id);
	if (res != ECommitActionResult::Success) {
		MessageConsole(FString::Format(TEXT("Action '{0}' could not be committed: {1}"), {
			*act.action_id.ToString(),
			*UEnum::GetDisplayValueAsText(res).ToString()
		}));
	}
}

void UAlibiTxtBasedCheats::MessageConsole(FString message) {
	_pc->ClientMessage(message);
	UE_LOG(LogTxtCheats, Log, TEXT("[TxtCheats] %s"), *message);
}
