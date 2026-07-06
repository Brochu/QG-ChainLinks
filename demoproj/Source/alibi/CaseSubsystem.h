// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AlibiEntityDefinitions.h"
#include "ChainSubsystem.h"
#include "MatrixSubsystem.h"
#include "CaseSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCase, Log, Log);

USTRUCT(BlueprintType)
struct FLabRequest {
	GENERATED_BODY()

	UPROPERTY()
	FName action_id;

	UPROPERTY()
	int32 block_started;
};

/**
 * The player's progress, kept as a diff against the authored case file
 * (Case File Data Inventory §12). Resetting a case = resetting this struct.
 */
USTRUCT(BlueprintType)
struct FCaseSaveState {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FName active_locid;

	UPROPERTY(SaveGame)
	int32 used_blocks = 0;

	UPROPERTY(SaveGame)
	TSet<FName> known_facts;

	UPROPERTY(SaveGame)
	TSet<FName> active_tags;

	UPROPERTY(SaveGame)
	TSet<FName> complete_actions;

	UPROPERTY(SaveGame)
	TArray<FLabRequest> active_lab_requests;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewLabRequest, FLabRequest&, request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlockSpent, int32, from, int32, to);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFactDiscovered, FName, fact_id, int32, when_block);

/**
 * Outcome of attempting to commit an action. `Success` (0) means it went through;
 * every other value is a distinct reason the commit was refused — suitable for
 * player feedback and for a future OnActionRejected event. The comment on each
 * entry is the rule that produces it.
 */
UENUM(BlueprintType)
enum class ECommitActionResult : uint8 {
	Success                   UMETA(DisplayName = "Success"),                      // committed; block(s) spent
	UnknownAction             UMETA(DisplayName = "Unknown Action"),               // no action with that id
	NotAtLocation             UMETA(DisplayName = "Not At Location"),              // action lives at a location the player isn't at
	PrerequisitesNotMet       UMETA(DisplayName = "Prerequisites Not Met"),        // prereq facts not all discovered (locked / secret)
	OutsideAvailabilityWindow UMETA(DisplayName = "Outside Availability Window"),  // current block outside the action's `available` range
	WrongBlockOfDay           UMETA(DisplayName = "Wrong Block Of Day"),           // current block-of-day not in `blocks_of_day`
	WrongLocationState        UMETA(DisplayName = "Wrong Location State"),         // current diorama state not in `location_states`
	NotEnoughBlocks           UMETA(DisplayName = "Not Enough Blocks"),            // cost would exceed the remaining deadline budget
	LabQueueFull              UMETA(DisplayName = "Lab Queue Full"),               // delayed request, but the lab queue is at capacity
	AlreadyCompleted          UMETA(DisplayName = "Already Completed"),            // non-repeatable action already taken
};

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
	// The static authored universe, loaded verbatim from the case file.
	// Never mutates during play — everything the player changes lives in `save`.
	UPROPERTY(Transient)
	FCaseFile file;

	// The player's diff against `authored`.
	UPROPERTY(SaveGame)
	FCaseSaveState save;
	// --------------------

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
	ECommitActionResult commit_action(FName action_id);
	// --------------------

	UPROPERTY(BlueprintAssignable)
	FOnNewLabRequest on_new_lab_request;

	UPROPERTY(BlueprintAssignable)
	FOnBlockSpent on_block_spent;

	UPROPERTY(BlueprintAssignable)
	FOnFactDiscovered on_fact_discovered;

private:
	void spend_blocks(int32 quantity);
	void discover_facts(const TArray<FName> &facts);
};
