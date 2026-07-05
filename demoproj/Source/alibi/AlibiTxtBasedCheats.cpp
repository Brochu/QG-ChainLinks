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

void UAlibiTxtBasedCheats::ExploreLocation() {
	MessageConsole(TEXT("List all elements in the room and possible actions ..."));

	if (_case_system) {
		MessageConsole(TEXT(" VALID CASE SUBSYSTEM ! "));
	}
	if (_chain_system) {
		MessageConsole(TEXT(" VALID CHAIN SUBSYSTEM ! "));
	}
	FCaseLocation loc = _case_system->locations[_case_system->active_locid];
	for (int32 i = 0; i < _case_system->actions.Num(); i++) {
		FCaseAction &act = _case_system->actions[i];

		if (act.location_id == loc.location_id) {
			//TODO: Keep working here to list locations, move and list actions at active location IDX
			MessageConsole(FString::Format(TEXT(" - [{0}][{1}] {2} / cost: {3}"), {
				*act.action_id.ToString(),
				*UEnum::GetValueAsString(act.verb),
				*act.label.ToString(),
				act.cost
			}));
		}
	}
}

void UAlibiTxtBasedCheats::MoveToLocation(int32 new_location_idx) {
	TArray<FCaseLocation*> accessible_locations;
}

void UAlibiTxtBasedCheats::ChooseAction(int32 action_idx) {
	FCaseLocation loc = _case_system->locations[_case_system->active_locid];
	TArray<FCaseAction*> actions_at_location;

	for (int32 i = 0; i < _case_system->actions.Num(); i++) {
		FCaseAction &act = _case_system->actions[i];

		if (act.location_id == loc.location_id) {
			actions_at_location.Add(&act);
		}
	}

	if (action_idx >= actions_at_location.Num()) {
		MessageConsole(FString::Format(TEXT("Could not choose action at index {0}; action count at location: {1}"), { action_idx, actions_at_location.Num()}));
		return;
	}

	FCaseAction &act = *actions_at_location[action_idx];
	MessageConsole(FString::Format(TEXT("Picked action: [{0}][{1}] {2} / cost: {3}"), {
		*act.action_id.ToString(),
		*UEnum::GetValueAsString(act.verb),
		*act.label.ToString(),
		act.cost
	}));
}

void UAlibiTxtBasedCheats::MessageConsole(FString message) {
	_pc->ClientMessage(message);
	UE_LOG(LogTxtCheats, Log, TEXT("[TxtCheats] %s"), *message);
}
