// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MatrixSubsystem.generated.h"

/**
 * Information matrix subsystem. This will keep track of the information that was found by investigation
 * Each data point needs to have a possibility of multiple conflicting information
 * This does not represent the truth of the virtual world, but the current knowledge of the investigators
 */

#define MAX_PEOPLE 255
#define MAX_LOCATIONS 255
#define MAX_OBJECTS 255

// Start with one enum, see if we can split it later?
UENUM()
enum class EDataKey {
	// PEOPLE KEYS
	PER_FIRSTNAME,
	PER_LASTNAME,
	PER_ADDRESS, // Could refer to location
	PER_PHONENUM,

	// LOCATIONS KEYS
	LOC_TYPE,
	LOC_ADDRESS,
	LOC_CITY,
	LOC_COUNTRY,

	// OBJECTS KEYS
	OBJ_TYPE,
	OBJ_NAME,
	OBJ_COLOR,
	OBJ_OWNER, // Could refer to people
};

USTRUCT(BlueprintType)
struct FDataPoint {
	// Represents a new value being added to the matrix, needs to have all the info required to place the
	// point at the right place in the matrix
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FDataEntry {
	// Represents an entry in the matrix, needs to track multiple conflicting values and history
	GENERATED_BODY()
};

UCLASS()
class ALIBI_API UMatrixSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	UFUNCTION(Category = "Alibi|Matrix")
	bool AddDataPoint(EDataKey key, FDataPoint point) {
		return false;
	}

	void ShowMatrix() {
		//TODO: Find a correct interface to communicate the whole matrix back to the game to show in the UI
	}

	UPROPERTY(Transient)
	FDataEntry people[MAX_PEOPLE];
	UPROPERTY(Transient)
	int32 num_peolpe = 0;

	UPROPERTY(Transient)
	FDataEntry locations[MAX_LOCATIONS];
	UPROPERTY(Transient)
	int32 num_locations = 0;

	UPROPERTY(Transient)
	FDataEntry objects[MAX_OBJECTS];
	UPROPERTY(Transient)
	int32 num_objects = 0;
};
