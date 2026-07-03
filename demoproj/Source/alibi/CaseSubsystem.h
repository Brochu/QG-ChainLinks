// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AlibiEntityDefinitions.h"
#include "ChainSubsystem.h"
#include "MatrixSubsystem.h"
#include "CaseSubsystem.generated.h"

USTRUCT()
struct FCaseSetting {
	GENERATED_BODY()

	UPROPERTY()
	FText date;

	UPROPERTY()
	FText region;
};

USTRUCT()
struct FCaseMetadata {
	GENERATED_BODY()

	UPROPERTY()
	int32 version;

	UPROPERTY()
	FName case_id;

	UPROPERTY()
	FText title;

	UPROPERTY()
	FCaseSetting setting;

	UPROPERTY()
	int32 deadline_days;

	UPROPERTY()
	int32 blocks_per_day;

	UPROPERTY()
	FName briefing_knot;

	UPROPERTY()
	TArray<FName> starting_facts;

	UPROPERTY()
	TArray<FName> starting_locations;

	UPROPERTY()
	int32 lab_queue_capacity;
};

USTRUCT()
struct FCaseFile {
	GENERATED_BODY()

	UPROPERTY()
	FCaseMetadata meta;
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
	FCaseFile current_case;

	UPROPERTY(Transient)
	TArray<FCaseLocation> locations;

	UPROPERTY(Transient)
	TArray<FCaseAction> actions;

	UPROPERTY(SaveGame)
	int32 action_points = 0;

	UPROPERTY(SaveGame)
	int32 active_locid = 0;

	UFUNCTION(BlueprintCallable)
	void load_case_file(FString case_path);
};
