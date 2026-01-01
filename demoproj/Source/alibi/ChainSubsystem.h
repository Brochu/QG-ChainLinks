// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MatrixSubsystem.h"
#include "ChainSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FChainBlank {
    GENERATED_BODY()

    FString blank_id;
    EEntityType entity_type;
    int32 entity_id;
    uint8 info_type_id;

    FString prompt_text;

    int32 player_fork_index;
};

USTRUCT(BlueprintType)
struct FChainStep {
    GENERATED_BODY()

    FString step_id;
    FString step_description;
    TArray<FChainBlank> blanks;
    int32 step_weight;
};

USTRUCT(BlueprintType)
struct FChainOfTruths {
    GENERATED_BODY()

    FString case_id;
    FString case_name;
    TArray<FChainStep> steps;
    int32 total_weight;
};

USTRUCT(BlueprintType)
struct FBlankEvaluation {
    GENERATED_BODY()

    FString blank_id;
    bool bIsCorrectEntry;
    int32 fork_selected;
    int32 correct_fork;
    int32 score_earned;
    int32 max_score;
};

USTRUCT(BlueprintType)
struct FStepEvaluation {
    GENERATED_BODY()

    FString step_id;
    int32 blanks_filled;
    int32 blanks_total;
    int32 score_earned;
    int32 max_score;
    TArray<FBlankEvaluation> blank_evaluations;
};

USTRUCT(BlueprintType)
struct FChainEvaluation {
    GENERATED_BODY()

    FString case_id;
    FString evaluation_grade;
    int32 total_score;
    int32 max_score;
    float score_percentage;
    int32 blanks_filled;
    int32 blanks_total;
    TArray<FStepEvaluation> step_evaluations;
};

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
    void Chain_LoadCase(FString case_id, FString case_name, TArray<FChainStep> steps);

    UFUNCTION(Category = "Alibi|Chain")
    void Chain_ClearCase();

    UFUNCTION(Category = "Alibi|Chain")
    bool Chain_HasActiveCase();

    // =============================
    // Step and Blank Queries
    // =============================

    UFUNCTION(Category = "Alibi|Chain")
    TArray<FChainStep> Chain_GetAllSteps();

    UFUNCTION(Category = "Alibi|Chain")
    FChainStep Chain_GetStep(int32 step_index);

    UFUNCTION(Category = "Alibi|Chain")
    int32 Chain_GetStepCount();

    UFUNCTION(Category = "Alibi|Chain")
    TArray<FChainBlank> Chain_GetBlanksForStep(int32 step_index);

    UFUNCTION(Category = "Alibi|Chain")
    FChainBlank Chain_GetBlank(int32 step_index, int32 blank_index);

    // =============================
    // Player Selection
    // =============================

    UFUNCTION(Category = "Alibi|Chain")
    void Chain_SelectBlank(int32 step_index, int32 blank_index);

    UFUNCTION(Category = "Alibi|Chain")
    void Chain_FillBlank(int32 step_index, int32 blank_index, int32 fork_index);

    UFUNCTION(Category = "Alibi|Chain")
    void Chain_ClearBlankSelection(int32 step_index, int32 blank_index);

    UFUNCTION(Category = "Alibi|Chain")
    bool Chain_IsBlankFilled(int32 step_index, int32 blank_index);

    // =============================
    // Progress Tracking
    // =============================

    UFUNCTION(Category = "Alibi|Chain")
    int32 Chain_GetFilledBlankCount();

    UFUNCTION(Category = "Alibi|Chain")
    int32 Chain_GetTotalBlankCount();

    UFUNCTION(Category = "Alibi|Chain")
    float Chain_GetProgressPercentage();

    UFUNCTION(Category = "Alibi|Chain")
    TArray<int32> Chain_GetIncompleteBlankIndices();

    // =============================
    // Evaluation
    // =============================

    UFUNCTION(Category = "Alibi|Chain")
    FChainEvaluation Chain_EvaluateCase();

    UFUNCTION(Category = "Alibi|Chain")
    FString Chain_GetGradeDisplayString(FChainEvaluation evaluation);

    // =============================
    // UI Helpers
    // =============================

    UFUNCTION(Category = "Alibi|Chain|UI")
    FString Chain_GetBlankDisplayText(int32 step_index, int32 blank_index);

    UFUNCTION(Category = "Alibi|Chain|UI")
    int32 Chain_GetStepScore(int32 step_index);

private:
    UPROPERTY()
    FChainOfTruths active_chain;

    int32 selected_step_index;
    int32 selected_blank_index;
};
