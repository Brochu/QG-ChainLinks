// Fill out your copyright notice in the Description page of Project Settings.

#include "CaseSubsystem.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogCase);

void UCaseSubsystem::load_case_file(FString case_path) {
	FString contents;
	if (!FFileHelper::LoadFileToString(contents, *case_path)) {
		UE_LOG(LogCase, Error, TEXT("load_case_file: could not read case file '%s' (file missing or unreadable)."), *case_path);
		return;
	}

	TSharedPtr<FJsonObject> root;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(contents);
	if (!FJsonSerializer::Deserialize(reader, root) || !root.IsValid()) {
		UE_LOG(LogCase, Error,
			TEXT("load_case_file: malformed JSON in '%s': %s"),
			*case_path, *reader->GetErrorMessage());
		return;
	}

	FCaseFile case_data;
	if (!FJsonObjectConverter::JsonObjectToUStruct(root.ToSharedRef(), &case_data)) {
		UE_LOG(LogCase, Error, TEXT("load_case_file: '%s' is valid JSON but does not match the case schema (field type mismatch?)."), *case_path);
		return;
	}

	if (case_data.locations.Num() == 0) {
		UE_LOG(LogCase, Error, TEXT("load_case_file: '%s' parsed but contains no locations; cannot start the case."), *case_path);
		return;
	}

	file = MoveTemp(case_data);
	save = FCaseSaveState();

	UE_LOG(LogCase, Log, TEXT("load_case_file: loaded '%s' — case '%s' (%d facts, %d actions, %d locations)."),
		*case_path,
		*file.meta.case_id.ToString(),
		file.facts.Num(),
		file.actions.Num(),
		file.locations.Num()
	);

	// Always start at first location in the case file
	save.active_locid = file.locations[0].location_id;
	discover_facts(file.meta.starting_facts);
}

const FCaseLocation &UCaseSubsystem::get_active_location() const {
	const FCaseLocation *active_loc = file.locations.FindByPredicate([this](const FCaseLocation &other) { return other.location_id == save.active_locid; });

	if (active_loc == nullptr) {
		// Invariant break: active_locid should always name a loaded location.
		UE_LOG(LogCase, Error,
			TEXT("get_active_location: active_locid '%s' is not a loaded location; falling back to the first."),
			*save.active_locid.ToString());
		active_loc = &file.locations[0];
	}
	return *active_loc;
}

FName UCaseSubsystem::get_location_state(FName loc_id) const {
	const FCaseLocation *found_loc= file.locations.FindByPredicate([this](const FCaseLocation &other) { return other.location_id == save.active_locid; });
	if (found_loc == nullptr) {
		return NAME_None;
	}

	const FCaseLocation &loc = *found_loc;
	FName state = loc.states[0].state_id;

	for (int32 i = 1; i < loc.states.Num(); i++) {
		if (all_facts_known(loc.states[i].when)) {
			state = loc.states[i].state_id;
		}
	}
	return state;
}

TArray<int32> UCaseSubsystem::list_location_idx() const {
	TArray<int32> results;

	for (int32 i = 0; i < file.locations.Num(); i++) {
		results.Add(i);
	}

	return results;
}

TArray<int32> UCaseSubsystem::list_action_idx(FName loc_id) const {
	TArray<int32> results;

	for (int32 i = 0; i < file.actions.Num(); i++) {
		if (file.actions[i].location_id != loc_id) {
			continue;
		}

		const EActionVisibility vis = action_visibility(file.actions[i]);
		if (vis == EActionVisibility::Absent || vis == EActionVisibility::Secret) {
			continue;
		}
		results.Add(i);
	}

	return results;
}

EActionVisibility UCaseSubsystem::action_visibility(const FCaseAction &act) const {
	const bool in_state =
		act.location_states.Num() == 0 ||
		act.location_states.Contains(get_location_state(act.location_id));

	const int32 from = act.available[0];
	const int32 to = act.available[1];
	const bool in_window = save.used_blocks >= from && ( to == -1 || save.used_blocks <= to);

	if (!in_state || !in_window) {
		return EActionVisibility::Absent;
	}

	if (!all_facts_known(act.prerequisites)) {
		return act.hidden ? EActionVisibility::Secret : EActionVisibility::Locked;
	}

	return EActionVisibility::Unlocked;
}

ECommitActionResult UCaseSubsystem::can_commit(const FCaseAction &act) const {
	if (act.location_id != save.active_locid) {
		return ECommitActionResult::NotAtLocation;
	}

	if (act.location_states.Num() > 0 && !act.location_states.Contains(get_location_state(act.location_id))) {
		return ECommitActionResult::WrongLocationState;
	}

	const int32 from = act.available[0];
	const int32 to = act.available[1];
	const bool in_window = save.used_blocks >= from && (to == -1 || save.used_blocks <= to);
	if (!in_window) {
		return ECommitActionResult::OutsideAvailabilityWindow;
	}

	if (!all_facts_known(act.prerequisites)) {
		return ECommitActionResult::PrerequisitesNotMet;
	}

	const int32 block_of_day = save.used_blocks % FMath::Max(1, file.meta.blocks_per_day);
	if (act.blocks_of_day.Num() > 0 && !act.blocks_of_day.Contains(block_of_day)) {
		return ECommitActionResult::WrongBlockOfDay;
	}

	if ((save.used_blocks + act.cost) > (file.meta.deadline_days * file.meta.blocks_per_day)) {
		return ECommitActionResult::NotEnoughBlocks;
	}

	if (act.verb == ECaseVerb::COLLECT && save.active_lab_requests.Num() >= file.meta.lab_queue_capacity) {
		return ECommitActionResult::LabQueueFull;
	}

	if (!act.repeatable && save.completed_actions.Contains(act.action_id)) {
		return ECommitActionResult::AlreadyCompleted;
	}

	return ECommitActionResult::Success;
}

bool UCaseSubsystem::move_location(FName new_loc_id) {
	//TODO: Overly simple for now, will need events and checks later to react to movement during a case
	save.active_locid = new_loc_id;
	return true;
}

ECommitActionResult UCaseSubsystem::commit_action(FName action_id) {
	const FCaseAction *chosen_action = file.actions.FindByPredicate([action_id](const FCaseAction &other) { return other.action_id == action_id; });
	if (chosen_action == nullptr) {
		return ECommitActionResult::UnknownAction;
	}

	const ECommitActionResult res = can_commit(*chosen_action);
	if (res != ECommitActionResult::Success) {
		return res;
	}

	if (chosen_action->verb == ECaseVerb::COLLECT) {
		// Lab request action; delayed results
		save.active_lab_requests.Push({ action_id, save.used_blocks });
		on_new_lab_request.Broadcast(save.active_lab_requests.Last());
		//TODO: Need to handle delayed action that are not lab requests as well
	}

	spend_blocks(chosen_action->cost);
	if (chosen_action->delay <= 0) {
		discover_facts(chosen_action->produces);
	}
	if (!chosen_action->repeatable) {
		save.completed_actions.Add(chosen_action->action_id);
	}
	return ECommitActionResult::Success;
}

void UCaseSubsystem::spend_blocks(int32 quantity) {
	int32 prev_blocks = save.used_blocks;
	save.used_blocks += quantity;

	//TODO: Handle time moving forward
	// Handle lab requests ending
	// Handle schedule entries

	on_block_spent.Broadcast(prev_blocks, save.used_blocks);
}

void UCaseSubsystem::discover_facts(const TArray<FName> &new_facts) {
	for (auto &fact : new_facts) {
		bool already_known = save.known_facts.Contains(fact);
		save.known_facts.Add(fact);

		if (!already_known) {
			on_fact_discovered.Broadcast(fact, save.used_blocks);
		}
	}
}

bool UCaseSubsystem::all_facts_known(const TArray<FName> &facts_to_check) const {
	for (auto &fact : facts_to_check) {
		if (!save.known_facts.Contains(fact)) {
			return false;
		}
	}

	return true;
}
