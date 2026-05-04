// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AlibiEntityDefinitions.h"
#include "MatrixSubsystem.h"
#include "CaseSubsystem.generated.h"

UENUM(BlueprintType)
enum class EEvidenceType : uint8 { EVI_TESTIMONY, EVI_PHYSICAL_TRACE, EVI_RECORD, EVI_DERIVED_SIGNAL, };
UENUM(BlueprintType)
enum class EReliabilityCategory : uint8 { RLY_DISPROVEN, RLY_UNRELIABLE, RLY_QUESTIONABLE, RLY_RELIABLE, RLY_VERIFIED, };

/// <summary>
/// Represents one evidence item between two persons of the case
/// </summary>
USTRUCT(BlueprintType)
struct FCaseEvidence {
	GENERATED_BODY()

	UPROPERTY()
	int32 id;

	UPROPERTY()
	EEvidenceType type;

	UPROPERTY()
	FString name;

	UPROPERTY()
	FString description;

	UPROPERTY()
	int32 found_at;

	UPROPERTY()
	TArray<FEntityRef> linked_to;

	UPROPERTY()
	int32 origin_event;

	UPROPERTY()
	EReliabilityCategory reliability;

	UPROPERTY()
	TArray<FDataPointInfo> matrix_updates;
};
// ----------------------------------------------------------------------------

USTRUCT()
struct FLabResults {
	GENERATED_BODY()
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEvidenceFound, int32, evidence_idx);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLabRequestComplete, int32, evidence_idx, FLabResults, results);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeBudgetChanged, int32, from, int32, to);

/**
 * Subsystem that will keep track of the current case metadata, resources, player actions on the world.
 * Will store the current status of the case's world.
 * The actions the player take will be reported and handled here
 *   - examine location/evidence
 *   - (start/stop) witness interview
 *   - request study of evidence
 *   - combine info/evidence together
 */
UCLASS()
class ALIBI_API UCaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnEvidenceFound OnEvidenceFound;

	UPROPERTY(BlueprintAssignable)
	FOnTimeBudgetChanged OnTimeBudgetChanged;

	UPROPERTY(Transient)
	TArray<FCasePerson> case_people;

	UPROPERTY(Transient)
	TArray<FCaseLocation> case_locations;

	UPROPERTY(Transient)
	TArray<FCaseEvent> case_events;

	UPROPERTY(Transient)
	TArray<FCaseRelation> case_relationship;
};
