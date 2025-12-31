// Fill out your copyright notice in the Description page of Project Settings.


#include "MatrixSubsystem.h"

void UMatrixSubsystem::Matrix_Init() {
	//TODO: Clear matrix data; prepare for next case
}

bool UMatrixSubsystem::Matrix_AddDataPoint(FDataKey key, FDataPoint point, EConflictResolution resolution) {
	//TODO: Traverse data matric find where to add the new data
	// If info already exists at correct position, react with resolution FORK or CORRECT
	// FORK:    Start a new branch, conflitcting data for one information cell
	// CORRECT: Update the history of the current latest data for the information, new info invalidating the latest one
	return false;
}

void UMatrixSubsystem::Matrix_DisplayData() {
	//TODO: Find a correct interface to communicate the whole matrix back to the game to show in the UI
}
