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
	_matrix_system = inst->GetSubsystem<UMatrixSubsystem>();
}

void UAlibiTxtBasedCheats::RemovedFromCheatManager_Implementation() {
	_pc = nullptr;

	_case_system = nullptr;
	_chain_system = nullptr;
	_matrix_system = nullptr;
}

void UAlibiTxtBasedCheats::ExploreLocation() {
	MessageConsole(TEXT("List all elements in the room and possible actions ..."));

	if (_case_system) {
		MessageConsole(TEXT(" VALID CASE SUBSYSTEM ! "));
	}
	if (_chain_system) {
		MessageConsole(TEXT(" VALID CHAIN SUBSYSTEM ! "));
	}
	if (_matrix_system) {
		MessageConsole(TEXT(" VALID MATRIX SUBSYSTEM ! "));
	}
}

void UAlibiTxtBasedCheats::MessageConsole(FString message) {
	_pc->ClientMessage(message);
	UE_LOG(LogTxtCheats, Log, TEXT("[TxtCheats] %s"), *message);
}
