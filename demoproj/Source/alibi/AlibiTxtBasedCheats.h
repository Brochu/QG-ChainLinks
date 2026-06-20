// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "AlibiTxtBasedCheats.generated.h"

class UCaseSubsystem;
class UChainSubsystem;
class UMatrixSubsystem;

DECLARE_LOG_CATEGORY_EXTERN(LogTxtCheats, Log, All);

/**
 * 
 */
UCLASS()
class ALIBI_API UAlibiTxtBasedCheats : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	// UCheatManagerExtension interface
	virtual void AddedToCheatManager_Implementation() override;
	virtual void RemovedFromCheatManager_Implementation() override;

	UFUNCTION(Exec)
	void ExploreLocation();

private:
	void MessageConsole(FString message);

	APlayerController *_pc;

	UCaseSubsystem *_case_system;
	UChainSubsystem *_chain_system;
	UMatrixSubsystem *_matrix_system;
};
