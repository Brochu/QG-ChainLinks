// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MatrixSubsystem.generated.h"

UENUM(BlueprintType)
enum class EEntityType : uint8 { PERSON, LOCATION, OBJECT, EVENT, RELATION, };

// PEOPLE ENTITIES ------------------------------------------------------------
UENUM(BlueprintType)
enum class EPersonInfoType : uint8 {
	PER_ID,
	PER_NAME,
	PER_ALIAS,
	PER_AGE,
	PER_SEX,
	PER_TRAITS, // map<string, string>
	PER_RESIDENCE,
	PER_PHONE,
	PER_EMAIL,
	PER_OCCUPATION,
	PER_DECISION_STYLE, // 0=extreme impulsive; 1=mild impulsive; 2=mild calculating; 3=extreme calculating
	PER_CONFLICT_STYLE, // 0=extreme confront; 1=mild confront; 2=mild secret; 3=extreme secret
};

UENUM(BlueprintType)
enum class EColorTrait : uint8 { COL_BLACK, COL_WHITE, COL_BROWN, COL_BLONDE, COL_RED, COL_BLUE, COL_GREEN, COL_YELLOW, };
UENUM(BlueprintType)
enum class EBuildTrait : uint8 { BLD_SLIM, BLD_THIN, BLD_AVERAGE, BLD_ATHLETIC, BLD_MUSCULAR, BLD_STOCKY, BLD_HUSKY, BLD_HEAVYSET, };
// ----------------------------------------------------------------------------

// LOCATIONS ENTITIES ---------------------------------------------------------
UENUM(BlueprintType)
enum class ELocationInfoType : uint8 {
	LOC_ID,
	LOC_TYPE,
	LOC_NAME_ADDRESS,
	LOC_OWNER,
	LOC_RESIDENTS,
	LOC_ACCESS_LEVEL,
	LOC_SECURITY,
	LOC_OP_HOURS,
	LOC_CAPACITY,
	LOC_CONNECTED_TO, // Is it really important to have this information in the matrix? Cannot really be wrong information, should it be in CaseSubsystem in a map
};

UENUM(BlueprintType)
enum class ELocationType : uint8 { LOCTYPE_RESIDENCE, LOCTYPE_COMMERCIAL, LOCTYPE_OUTDOOR, LOCTYPE_TRANSPORT, LOCTYPE_INDUSTRIAL, };
UENUM(BlueprintType)
enum class EAccessLevel : uint8 { ACCESS_PUBLIC, ACCESS_SEMIPUBLIC, ACCESS_PRIVATE, ACCESS_RESTRICTED, };
UENUM(BlueprintType)
enum class ESecurityLevel : uint8 { SEC_NONE, SEC_LOW, SEC_HIGH, };
UENUM(BlueprintType)
enum class ELocationCapacity : uint8 { CAP_LOW, CAP_MEDIUM, CAP_HIGH, };
// ----------------------------------------------------------------------------

// OBJECTS ENTITIES -----------------------------------------------------------
UENUM(BlueprintType)
enum class EObjectInfoType : uint8 {
	OBJ_ID,
	OBJ_NAME,
	OBJ_TYPE,
	OBJ_SERIAL_NUM,
	OBJ_SIZE,
	OBJ_COLOR,
	OBJ_MATERIAL,
	OBJ_CONDITION,
	OBJ_OWNER,
	OBJ_PORTABILITY,
	OBJ_ORIGIN,
};

// TODO: need to define more enums for physical traits
// ----------------------------------------------------------------------------

// EVENTS ENTITIES ------------------------------------------------------------
UENUM(BlueprintType)
enum class EEventInfoType : uint8 {
	EVT_ID,
	EVT_TYPE,
	EVT_DATE_START,
	EVT_DATE_END,
	EVT_LOCATION,
	EVT_PARTICIPANTS,
	EVT_PARENTS, // parent events
	EVT_VISIBILITY,
};

UENUM(BlueprintType)
enum class EEventVisibility : uint8 { VIS_PUBLIC, VIS_SEMIPUBLIC, VIS_PRIVATE, VIS_SECRET, };
// ----------------------------------------------------------------------------

// RELATIONS ENTITIES ---------------------------------------------------------
UENUM(BlueprintType)
enum class ERelationsInfoType : uint8 {
	REL_ID,
	REL_TYPE,
	REL_PERSONA,
	REL_PERSONB,
};

UENUM(BlueprintType)
enum class ERelationType : uint8 {
	RELTYPE_SPOUSE,
	RELTYPE_EX_PARTNER,
	RELTYPE_ROMANTIC_PARTNER,
	RELTYPE_SIBLING,
	RELTYPE_PARENT,
	RELTYPE_CHILD,
	RELTYPE_FRIEND,
	RELTYPE_ACQUAINTANCE,
	RELTYPE_NEIGHBOR,
	RELTYPE_COWORKER,
	RELTYPE_EMPLOYER,
	RELTYPE_CREDITOR,
	RELTYPE_RIVAL,
	RELTYPE_BLACKMAILER,
};
// ----------------------------------------------------------------------------

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

	FDataKey() : key(0) { }
};

/// <summary>
/// Represents a cell of information in the matrix
/// </summary>
USTRUCT(BlueprintType)
struct FDataPoint {
	GENERATED_BODY()

	FString str_value;
	int int_value;

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
	GENERATED_BODY()

	int32 entity_id;
	FDataSource source;
	EReliabilityCategory reliability;
	FDataPoint data;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatrixCellUpdated, FDataPoint, new_point);

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

	// =============================

	UFUNCTION(Category = "Alibi|Matrix")
	int32 Matrix_GetForkCount(EEntityType entity_type, int32 entity_id, uint8 info_type_id);

	UFUNCTION(Category = "Alibi|Matrix")
	TArray<int32> Matrix_GetAllEntityIds(EEntityType entity_type);

	UPROPERTY(BlueprintAssignable)
	FOnMatrixCellUpdated OnMatrixCellUpdated;

private:
	UPROPERTY(Transient)
	TMap<uint64, FDataEntry> sparse_entries;
};
