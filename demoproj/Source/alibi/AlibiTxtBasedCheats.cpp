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
	//_matrix_system = inst->GetSubsystem<UMatrixSubsystem>();
}

void UAlibiTxtBasedCheats::RemovedFromCheatManager_Implementation() {
	_pc = nullptr;

	_case_system = nullptr;
	_chain_system = nullptr;
	//_matrix_system = nullptr;
}

void UAlibiTxtBasedCheats::ListLocations() {
	MessageConsole(FString::Format(TEXT("All available locations in the case: {0}"), { *_case_system->meta.title.ToString()}));

	TArray<int32> location_idx = _case_system->list_location_idx();
	for (int32 i = 0; i < location_idx.Num(); i++) {
		FCaseLocation &loc = _case_system->locations[location_idx[i]];

		MessageConsole(FString::Format(TEXT(" - {{0}} [{1}] {2}"), { i, *loc.location_id.ToString(), *loc.name.ToString() }));
	}
}

void UAlibiTxtBasedCheats::ExploreLocation() {
	FCaseLocation &loc = _case_system->get_active_location();
	MessageConsole(FString::Format(TEXT("All available actions at {0}"), { *loc.name.ToString() }));

	TArray<int32> action_idx = _case_system->list_action_idx(loc.location_id);
	for (int32 i = 0; i < action_idx.Num(); i++) {
		FCaseAction &act = _case_system->actions[action_idx[i]];

		MessageConsole(FString::Format(TEXT(" - {{0}} [{1}][{2}] {3} / cost: {4}"), {
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

	FName target_loc_id = _case_system->locations[location_idx[new_location_idx]].location_id;
	if (!_case_system->move_location(target_loc_id)) {
		MessageConsole(FString::Format(TEXT("Invalid move: could not move to '{0}'."), { *target_loc_id.ToString() }));
	}
}

void UAlibiTxtBasedCheats::ChooseAction(int32 action_idx) {
	// Index-based because it's easier to type on cli. Will be ID-based in actual game with real UX
	FCaseLocation &loc = _case_system->get_active_location();
	TArray<int32> actions_at_location = _case_system->list_action_idx(loc.location_id);

	if (action_idx < 0 || action_idx >= actions_at_location.Num()) {
		MessageConsole(FString::Format(TEXT("Could not choose action at index {0}; action count at location: {1}"), { action_idx, actions_at_location.Num()}));
		return;
	}

	FCaseAction &act = _case_system->actions[actions_at_location[action_idx]];
	MessageConsole(FString::Format(TEXT("Picked action: [{0}][{1}] {2} / cost: {3}"), {
		*act.action_id.ToString(),
		*UEnum::GetValueAsString(act.verb),
		*act.label.ToString(),
		act.cost
	}));
	if (!_case_system->commit_action(act.action_id)) {
		MessageConsole(FString::Format(TEXT("Action '{0}' could not be committed."), { *act.action_id.ToString() }));
	}
}

void UAlibiTxtBasedCheats::MessageConsole(FString message) {
	_pc->ClientMessage(message);
	UE_LOG(LogTxtCheats, Log, TEXT("[TxtCheats] %s"), *message);
}
