// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CaseSubsystem.generated.h"

USTRUCT()
struct FLabResults {
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEvidenceFound, int, evidence_idx);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParam(FOnLabRequestComplete, int, evidence_idx, FLabResults, results);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeBudgetChanged, int, from, int, to);

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
};
