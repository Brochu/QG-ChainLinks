// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AlibiEntityDefinitions.h"
#include "MatrixSubsystem.generated.h"

//TODO: Rethink out OBJECTs and CLUEs releate to each other and how they are stored alongside the matrix
//      We don't want clues in the matrix as they are unlocked and used a requirements for others, they won't be corrected / updated as the case progresses

UENUM(BlueprintType)
enum class EEntityType : uint8 { PERSON, LOCATION, EVENT, RELATION, };

USTRUCT(BlueprintType)
struct FEntityRef {
	GENERATED_BODY()

	EEntityType entity_type;
	int32 entity_id;
};

USTRUCT()
struct FDataKey {
	GENERATED_BODY()

	union {
		struct {
			EEntityType entity_type;
			int32 entity_id;
			int16 pad0;
		};
		uint64 key;
	};

	FDataKey() : key(0) { }
};

/// <summary>
/// Represents a cell of information in the matrix
/// </summary>
USTRUCT(BlueprintType)
struct FDataPoint {
	GENERATED_BODY()

	FString str_value;
	int32 int_value;

	int64 timestamp;
};

USTRUCT(BlueprintType)
struct FDataPointInfo {
	GENERATED_BODY()

	int32 clue_id;
	FDataKey entity_key;
	FDataPoint data;
};

/// <summary>
/// Represents one of many conflicting informations for one given entity info and it's history over time with corrections
/// </summary>
USTRUCT(BlueprintType)
struct FDataFork {
	GENERATED_BODY()

	int32 clue_id;
	TArray<FDataPoint> history;
};

/// <summary>
/// Represents an entry in the matrix, needs to track multiple conflicting values and history
/// </summary>
USTRUCT(BlueprintType)
struct FDataEntry {
	GENERATED_BODY()

	TArray<FDataFork> forks;
};

/// <summary>
/// Information matrix subsystem. This will keep track of the information that was found by investigation
/// Each data point needs to have a possibility of multiple conflicting information
/// This does not represent the truth of the virtual world, but the current knowledge of the investigators
/// </summary>
UCLASS()
class ALIBI_API UMatrixSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Alibi|Matrix")
	void Matrix_InitNewCase();

	UFUNCTION(Category = "Alibi|Matrix")
	static FDataKey Matrix_CreateKey(EEntityType entity_type, int32 entity_id, uint8 info_type_id);

	UFUNCTION(Category = "Alibi|Matrix")
	int32 Matrix_NewDataPoint(FDataPointInfo info);

	// =============================

	UFUNCTION(Category = "Alibi|Matrix")
	FDataPoint Matrix_GetCurrentValue(EEntityType entity_type, int32 entity_id, uint8 info_type_id, int32 fork_index);

	UFUNCTION(Category = "Alibi|Matrix")
	FDataEntry Matrix_GetDataEntry(EEntityType entity_type, int32 entity_id, uint8 info_type_id);

	UFUNCTION(Category = "Alibi|Matrix")
	TArray<FDataPoint> Matrix_GetDataHistory(EEntityType entity_type, int32 entity_id, uint8 info_type_id, int32 fork_index);

	UFUNCTION(Category = "Alibi|Matrix")
	TArray<FDataPoint> Matrix_GetAllForksLatest(EEntityType entity_type, int32 entity_id, uint8 info_type_id);

	UFUNCTION(Category = "Alibi|Matrix")
	TArray<FDataEntry> Matrix_GetAllDataForEntity(EEntityType entity_type, int32 entity_id);

	// =============================

	UFUNCTION(Category = "Alibi|Matrix")
	int32 Matrix_GetForkCount(EEntityType entity_type, int32 entity_id, uint8 info_type_id);

	UFUNCTION(Category = "Alibi|Matrix")
	TArray<int32> Matrix_GetAllEntityIds(EEntityType entity_type);

	UPROPERTY(Transient)
	TArray<int32> discovered_clues;

private:
	UPROPERTY(Transient)
	TMap<uint64, FDataEntry> sparse_entries;
};
