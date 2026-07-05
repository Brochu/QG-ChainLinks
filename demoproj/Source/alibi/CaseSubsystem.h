// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AlibiEntityDefinitions.h"
#include "ChainSubsystem.h"
#include "MatrixSubsystem.h"
#include "CaseSubsystem.generated.h"

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
	// Current case data
	UPROPERTY(Transient)
	FCaseMetadata meta;

	UPROPERTY(Transient)
	TArray<FCaseGlossaryEntry> glossary;

	UPROPERTY(Transient)
	TArray<FCaseFact> facts;

	UPROPERTY(Transient)
	TArray<FCaseContradiction> contradictions;

	UPROPERTY(Transient)
	TArray<FCaseLocation> locations;

	UPROPERTY(Transient)
	TArray<FCaseAction> actions;

	UPROPERTY(Transient)
	TArray<FCaseInterview> interviews;

	UPROPERTY(Transient)
	TArray<FCaseScheduleEntry> schedule;

	UPROPERTY(Transient)
	FCaseReconstruction reconstruction;

	UPROPERTY(Transient)
	TArray<FOutcomeTier> outcome_tiers;
	// --------------------

	UPROPERTY(SaveGame)
	FName active_locid;

	UPROPERTY(SaveGame)
	int32 action_points = 0;

	UPROPERTY(SaveGame)
	TSet<FName> known_facts = {};

	UPROPERTY(SaveGame)
	TSet<FName> active_tags = {};

	UFUNCTION(BlueprintCallable)
	void load_case_file(FString case_path);

	UFUNCTION(BlueprintCallable)
	FCaseLocation &get_active_location();

	UFUNCTION(BlueprintCallable)
	TArray<int32> list_location_idx();

	UFUNCTION(BlueprintCallable)
	TArray<int32> list_action_idx(FName loc_id);

	UFUNCTION(BlueprintCallable)
	bool move_location(FName new_loc_id);

	UFUNCTION(BlueprintCallable)
	bool commit_action(FName action_id);
};
