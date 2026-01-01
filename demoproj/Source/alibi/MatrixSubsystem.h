// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MatrixSubsystem.generated.h"

UENUM(BlueprintType)
enum class EEntityType : uint8 {
	PERSON,
	LOCATION,
	OBJECT,
	EVENT,
};

UENUM(BlueprintType)
enum class EPersonInfoType : uint8 {
	PER_NAME,
	PER_ALIAS,
	PER_AGE,
	PER_HEIGHT,
	PER_BUILD,
	PER_HAIR_COLOR,
	PER_MARKS,
	PER_ADDRESS,
	PER_PHONE,
	PER_EMAIL,
	PER_OCCUPATION,
	PER_ROLE,
	PER_ALIBI,
	PER_CRIME_HISTORY,
};

UENUM(BlueprintType)
enum class ELocationInfoType : uint8 {
	LOC_NAME,
	LOC_ADDRESS,
	LOC_TYPE,
	LOC_OWNER,
	LOC_RESIDENTS,
	LOC_ACCESS_CONTROL,
	LOC_OP_HOURS,
};

UENUM(BlueprintType)
enum class EObjectInfoType : uint8 {
	OBJ_NAME,
	OBJ_TYPE,
	OBJ_SERIAL_NUM,
	OBJ_SIZE,
	OBJ_COLOR,
	OBJ_MATERIAL,
	OBJ_CONDITION,
	OBJ_OWNER,
	OBJ_LOCATION,
};

UENUM(BlueprintType)
enum class EEventInfoType : uint8 {
	EVT_TYPE,
	EVT_DATE_START,
	EVT_DATE_END,
	EVT_LOCATION,
	EVT_PARTICIPANTS,
	EVT_WITNESSES,
	EVT_DESCRIPTION,
	EVT_STATUS,
};

USTRUCT()
struct FDataKey {
	GENERATED_BODY()

	union {
		struct {
			EEntityType entity_type;
			union {
				EPersonInfoType person_info_type;
				ELocationInfoType location_info_type;
				EObjectInfoType object_info_type;
				EEventInfoType event_info_type;
			};
			int16 pad0;
			int32 entity_id;
		};
		uint64 key;
	};
};

/// <summary>
/// Represents a cell of information in the matrix
/// </summary>
USTRUCT(BlueprintType)
struct FDataPoint {
	GENERATED_BODY()

	FString value;
	int64 timestamp;
};

UENUM(BlueprintType)
enum class EDataSourceType : uint8 {
	WITNESS,
	FORENSIC,
	DOCUMENT,
	CAMERA,
};

UENUM(BlueprintType)
enum class EReliabilityCategory : uint8 {
    DISPROVEN,
    UNRELIABLE,
    QUESTIONABLE,
    RELIABLE,
    VERIFIED,
};

USTRUCT(BlueprintType)
struct FDataSource {
	GENERATED_BODY()

	EDataSourceType src_type;
	int32 src_id;
	FString src_name;
};

/// <summary>
/// Represents one of many conflicting informations for one given entity info and it's history over time with corrections
/// </summary>
USTRUCT(BlueprintType)
struct FDataFork {
	GENERATED_BODY()
	//TODO :Add tagging features for fully confirmed forks / fully disproven forks

	FDataSource source;
	EReliabilityCategory reliability;

	TArray<FDataPoint> history;
};

/// <summary>
/// Represents an entry in the matrix, needs to track multiple conflicting values and history
/// </summary>
USTRUCT(BlueprintType)
struct FDataEntry {
	GENERATED_BODY()
	//TODO :Add tagging features for their value in the chain of event

	TArray<FDataFork> forks;
};

USTRUCT(BlueprintType)
struct FDataPointInfo {
	GENERATED_BODY();

	int32 entity_id;
	FDataSource source;
	EReliabilityCategory reliability;
	FDataPoint data;
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

	//TODO: Add more function to get/set information
	// DO NOT FORCE THE USER TO INTERACT WITH THE KEY STRUCTS; combine it into a key for them in the function
	UFUNCTION(Category = "Alibi|Matrix")
	void Matrix_InitNewCase();

	UFUNCTION(Category = "Alibi|Matrix")
	int32 Matrix_NewPersonDataPoint(EPersonInfoType info_type, FDataPointInfo info);

	UFUNCTION(Category = "Alibi|Matrix")
	int32 Matrix_NewLocationDataPoint(ELocationInfoType info_type, FDataPointInfo info);

	UFUNCTION(Category = "Alibi|Matrix")
	int32 Matrix_NewObjectDataPoint(EObjectInfoType info_type, FDataPointInfo info);

	UFUNCTION(Category = "Alibi|Matrix")
	int32 Matrix_NewEventDataPoint(EEventInfoType info_type, FDataPointInfo info);

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

	UPROPERTY(Transient)
	TMap<uint64, FDataEntry> sparse_entries;
};
