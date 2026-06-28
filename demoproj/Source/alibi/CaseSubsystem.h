// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AlibiEntityDefinitions.h"
#include "ChainSubsystem.h"
#include "MatrixSubsystem.h"
#include "CaseSubsystem.generated.h"

USTRUCT()
struct FCaseData {
	GENERATED_BODY()

	UPROPERTY()
	FText name;

	UPROPERTY()
	FText desc;

	UPROPERTY()
	FName intro_knot;

	UPROPERTY()
	int32 version;
};

DECLARE_LOG_CATEGORY_EXTERN(LogCase, Log, Log);

/**
 * Subsystem that will keep track of the current case metadata, resources, player actions on the world.
 * Will store the current status of the case's world.
 * The actions the player take will be reported and handled here
 *   - examine location/clue
 *   - (start/stop) witness interview
 *   - request study of clue
 *   - combine info/clue together
 */
UCLASS()
class ALIBI_API UCaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FCaseData current_case;

	UPROPERTY(SaveGame)
	int32 actions_used = 0;

	UFUNCTION(BlueprintCallable)
	void load_case_file(FString case_path);
};
