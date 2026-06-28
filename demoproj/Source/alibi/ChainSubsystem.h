// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ChainSubsystem.generated.h"

/// <summary>
/// Chain of truths subsystem. This manages the narrative solution path for each case
/// and handles player progress tracking and end-game grading.
/// </summary>
UCLASS()
class ALIBI_API UChainSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // =============================
    // Case Management
    // =============================

    UFUNCTION(Category = "Alibi|Chain")
    void chain_init(FString case_id, FString case_name);

    UFUNCTION(Category = "Alibi|Chain")
    void chain_clear();

    UFUNCTION(Category = "Alibi|Chain")
    bool chain_is_active();
};
