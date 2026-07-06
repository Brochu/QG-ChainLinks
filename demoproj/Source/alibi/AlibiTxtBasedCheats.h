// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "AlibiTxtBasedCheats.generated.h"

class UCaseSubsystem;
class UChainSubsystem;
//class UMatrixSubsystem;

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
	void CaseStatus();

	UFUNCTION(Exec)
	void ShowCaseMeta();

	UFUNCTION(Exec)
	void ListLocations();

	UFUNCTION(Exec)
	void ExploreLocation();

	UFUNCTION(Exec)
	void MoveToLocation(int32 new_location_idx);

	UFUNCTION(Exec)
	void ChooseAction(int32 action_idx);

private:
	void MessageConsole(FString message);

	UPROPERTY(Transient)
	APlayerController *_pc;

	UPROPERTY(Transient)
	UCaseSubsystem *_case_system;
	UPROPERTY(Transient)
	UChainSubsystem *_chain_system;
};
