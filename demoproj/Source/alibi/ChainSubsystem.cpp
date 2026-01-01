// Fill out your copyright notice in the Description page of Project Settings.


#include "ChainSubsystem.h"
#include "MatrixSubsystem.h"

void UChainSubsystem::Chain_LoadCase(FString case_id, FString case_name, TArray<FChainStep> steps) {
    active_chain.case_id = case_id;
    active_chain.case_name = case_name;
    active_chain.steps = steps;

    // Calculate total weight
    active_chain.total_weight = 0;
    for (FChainStep& step : active_chain.steps) {
        active_chain.total_weight += step.step_weight;
    }

    // Initialize all blanks with unfilled state
    for (FChainStep& step : active_chain.steps) {
        for (FChainBlank& blank : step.blanks) {
            blank.player_fork_index = -1;
        }
    }

    selected_step_index = -1;
    selected_blank_index = -1;
}

void UChainSubsystem::Chain_ClearCase() {
    active_chain = FChainOfTruths();
    selected_step_index = -1;
    selected_blank_index = -1;
}

bool UChainSubsystem::Chain_HasActiveCase() {
    return !active_chain.case_id.IsEmpty();
}

TArray<FChainStep> UChainSubsystem::Chain_GetAllSteps() {
    return active_chain.steps;
}

FChainStep UChainSubsystem::Chain_GetStep(int32 step_index) {
    if (step_index >= 0 && step_index < active_chain.steps.Num()) {
        return active_chain.steps[step_index];
    }
    return FChainStep();
}

int32 UChainSubsystem::Chain_GetStepCount() {
    return active_chain.steps.Num();
}

TArray<FChainBlank> UChainSubsystem::Chain_GetBlanksForStep(int32 step_index) {
    if (step_index >= 0 && step_index < active_chain.steps.Num()) {
        return active_chain.steps[step_index].blanks;
    }
    return TArray<FChainBlank>();
}

FChainBlank UChainSubsystem::Chain_GetBlank(int32 step_index, int32 blank_index) {
    if (step_index >= 0 && step_index < active_chain.steps.Num()) {
        TArray<FChainBlank>& blanks = active_chain.steps[step_index].blanks;
        if (blank_index >= 0 && blank_index < blanks.Num()) {
            return blanks[blank_index];
        }
    }
    return FChainBlank();
}

void UChainSubsystem::Chain_SelectBlank(int32 step_index, int32 blank_index) {
    if (step_index >= 0 && step_index < active_chain.steps.Num()) {
        TArray<FChainBlank>& blanks = active_chain.steps[step_index].blanks;
        if (blank_index >= 0 && blank_index < blanks.Num()) {
            selected_step_index = step_index;
            selected_blank_index = blank_index;
        }
    }
}

void UChainSubsystem::Chain_FillBlank(int32 step_index, int32 blank_index, int32 fork_index) {
    if (step_index >= 0 && step_index < active_chain.steps.Num()) {
        TArray<FChainBlank>& blanks = active_chain.steps[step_index].blanks;
        if (blank_index >= 0 && blank_index < blanks.Num()) {
            blanks[blank_index].player_fork_index = fork_index;
        }
    }
}

void UChainSubsystem::Chain_ClearBlankSelection(int32 step_index, int32 blank_index) {
    if (step_index >= 0 && step_index < active_chain.steps.Num()) {
        TArray<FChainBlank>& blanks = active_chain.steps[step_index].blanks;
        if (blank_index >= 0 && blank_index < blanks.Num()) {
            blanks[blank_index].player_fork_index = -1;
        }
    }
}

bool UChainSubsystem::Chain_IsBlankFilled(int32 step_index, int32 blank_index) {
    if (step_index >= 0 && step_index < active_chain.steps.Num()) {
        TArray<FChainBlank>& blanks = active_chain.steps[step_index].blanks;
        if (blank_index >= 0 && blank_index < blanks.Num()) {
            return blanks[blank_index].player_fork_index >= 0;
        }
    }
    return false;
}

int32 UChainSubsystem::Chain_GetFilledBlankCount() {
    int32 count = 0;
    for (const FChainStep& step : active_chain.steps) {
        for (const FChainBlank& blank : step.blanks) {
            if (blank.player_fork_index >= 0) {
                count++;
            }
        }
    }
    return count;
}

int32 UChainSubsystem::Chain_GetTotalBlankCount() {
    int32 count = 0;
    for (const FChainStep& step : active_chain.steps) {
        count += step.blanks.Num();
    }
    return count;
}

float UChainSubsystem::Chain_GetProgressPercentage() {
    int32 total = Chain_GetTotalBlankCount();
    if (total == 0) {
        return 0.0f;
    }
    return (float)Chain_GetFilledBlankCount() / (float)total * 100.0f;
}

TArray<int32> UChainSubsystem::Chain_GetIncompleteBlankIndices() {
    TArray<int32> indices;
    int32 blank_counter = 0;

    for (int32 step_idx = 0; step_idx < active_chain.steps.Num(); step_idx++) {
        for (int32 blank_idx = 0; blank_idx < active_chain.steps[step_idx].blanks.Num(); blank_idx++) {
            if (active_chain.steps[step_idx].blanks[blank_idx].player_fork_index < 0) {
                indices.Add(blank_counter);
            }
            blank_counter++;
        }
    }

    return indices;
}

FChainEvaluation UChainSubsystem::Chain_EvaluateCase() {
    //TODO: This is way too complicated, we probably don't need the weight concept at all in this case
    FChainEvaluation evaluation;
    evaluation.case_id = active_chain.case_id;
    evaluation.total_score = 0;
    evaluation.max_score = 0;
    evaluation.step_evaluations.Empty();

    UMatrixSubsystem* matrix = GetGameInstance()->GetSubsystem<UMatrixSubsystem>();

    int32 total_score = 0;
    int32 max_possible_score = 0;
    int32 total_filled = 0;
    int32 total_blanks = 0;

    for (int32 step_idx = 0; step_idx < active_chain.steps.Num(); step_idx++) {
        const FChainStep& step = active_chain.steps[step_idx];
        FStepEvaluation step_eval;
        step_eval.step_id = step.step_id;
        step_eval.blanks_filled = 0;
        step_eval.blanks_total = step.blanks.Num();
        step_eval.score_earned = 0;
        step_eval.max_score = 0;
        step_eval.blank_evaluations.Empty();

        int32 step_max_score = step.step_weight * step.blanks.Num();

        for (int32 blank_idx = 0; blank_idx < step.blanks.Num(); blank_idx++) {
            const FChainBlank& blank = step.blanks[blank_idx];
            FBlankEvaluation blank_eval;
            blank_eval.blank_id = blank.blank_id;
            blank_eval.fork_selected = -1;
            blank_eval.correct_fork = -1;
            blank_eval.score_earned = 0;
            blank_eval.max_score = step.step_weight;

            total_blanks++;

            if (blank.player_fork_index >= 0) {
                total_filled++;
                blank_eval.fork_selected = blank.player_fork_index;

                // Get the data entry from matrix to find correct fork
                FDataEntry entry = matrix->Matrix_GetDataEntry(
                    blank.entity_type,
                    blank.entity_id,
                    blank.info_type_id
                );

                // Find the fork with the correct value
                // For now, we assume the first fork is the correct one
                // This will be refined when case generation is implemented
                if (entry.forks.Num() > 0) {
                    blank_eval.correct_fork = 0;

                    // Check if player selected the correct fork
                    if (blank.player_fork_index == blank_eval.correct_fork) {
                        blank_eval.bIsCorrectEntry = true;
                        blank_eval.score_earned = step.step_weight;
                    } else {
                        // Player selected a different fork
                        // Check if the selected fork has the same source
                        if (blank.player_fork_index >= 0 && blank.player_fork_index < entry.forks.Num()) {
                            const FDataFork& selected_fork = entry.forks[blank.player_fork_index];
                            const FDataFork& correct_fork = entry.forks[blank_eval.correct_fork];

                            if (selected_fork.source.src_type == correct_fork.source.src_type &&
                                selected_fork.source.src_id == correct_fork.source.src_id) {
                                // Same source, different data - partial credit
                                blank_eval.bIsCorrectEntry = true;
                                blank_eval.score_earned = step.step_weight * 0.7f;
                            } else {
                                // Different source - less credit
                                blank_eval.bIsCorrectEntry = false;
                                blank_eval.score_earned = step.step_weight * 0.5f;
                            }
                        } else {
                            blank_eval.bIsCorrectEntry = false;
                            blank_eval.score_earned = 0;
                        }
                    }
                } else {
                    blank_eval.bIsCorrectEntry = false;
                    blank_eval.score_earned = 0;
                }

                step_eval.score_earned += blank_eval.score_earned;
            } else {
                blank_eval.bIsCorrectEntry = false;
            }

            step_eval.blank_evaluations.Add(blank_eval);
            total_score += blank_eval.score_earned;
        }

        step_eval.max_score = step_max_score;
        evaluation.step_evaluations.Add(step_eval);
        max_possible_score += step_max_score;
    }

    evaluation.total_score = total_score;
    evaluation.max_score = max_possible_score;
    evaluation.score_percentage = max_possible_score > 0 ?
        (float)total_score / (float)max_possible_score * 100.0f : 0.0f;
    evaluation.blanks_filled = total_filled;
    evaluation.blanks_total = total_blanks;
    evaluation.evaluation_grade = Chain_GetGradeDisplayString(evaluation);

    return evaluation;
}

FString UChainSubsystem::Chain_GetGradeDisplayString(FChainEvaluation evaluation) {
    float percentage = evaluation.score_percentage;

    if (percentage >= 95.0f) {
        return TEXT("CASE CLOSED: CONFIRMED");
    } else if (percentage >= 80.0f) {
        return TEXT("CASE CLOSED: VERIFIED");
    } else if (percentage >= 60.0f) {
        return TEXT("PROBABLE CAUSE ESTABLISHED");
    } else if (percentage >= 40.0f) {
        return TEXT("INVESTIGATION INCONCLUSIVE");
    } else if (percentage >= 20.0f) {
        return TEXT("LEADS EXHAUSTED");
    } else {
        return TEXT("CASE FAILED");
    }
}

FString UChainSubsystem::Chain_GetBlankDisplayText(int32 step_index, int32 blank_index) {
    FChainBlank blank = Chain_GetBlank(step_index, blank_index);
    if (blank.prompt_text.IsEmpty()) {
        return TEXT("[Select evidence]");
    }

    FString status = TEXT("");
    if (blank.player_fork_index >= 0) {
        status = TEXT(" [FILLED]");
    }

    return blank.prompt_text + status;
}

int32 UChainSubsystem::Chain_GetStepScore(int32 step_index) {
    if (step_index >= 0 && step_index < active_chain.steps.Num()) {
        const FChainStep& step = active_chain.steps[step_index];
        int32 score = 0;

        for (const FChainBlank& blank : step.blanks) {
            if (blank.player_fork_index >= 0) {
                score += step.step_weight;
            }
        }

        return score;
    }
    return 0;
}
