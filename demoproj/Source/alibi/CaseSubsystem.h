// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CaseSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEvidenceFound, FString, evidence_name);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeBudgetChanged, int, from, int, to);

/**
 * 
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
