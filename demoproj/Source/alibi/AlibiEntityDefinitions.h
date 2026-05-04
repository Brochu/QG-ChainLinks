#pragma once

#include "CoreMinimal.h"
#include "AlibiEntityDefinitions.generated.h"

/// <summary>
/// All possiblie matrix attributes for people in the case
/// </summary>
UENUM(BlueprintType)
enum class EPersonInfoType : uint8 {
	PER_ID,
	PER_NAME,
	PER_ALIAS,
	PER_AGE,
	PER_SEX,
	PER_EYECOLOR,
	PER_HAIRCOLOR,
	PER_BUILD,
	PER_RESIDENCE,
	PER_PHONE,
	PER_EMAIL,
	PER_OCCUPATION,
	PER_DECISION_STYLE,
	PER_CONFLICT_STYLE,
};

UENUM(BlueprintType)
enum class ESex : uint8 { SEX_M, SEX_F, };
UENUM(BlueprintType)
enum class EEyeColor : uint8 { EYE_BLACK, EYE_BROWN, EYE_BLONDE, EYE_RED, EYE_BLUE, EYE_GREEN, EYE_YELLOW, };
UENUM(BlueprintType)
enum class EHairColor : uint8 { HAIR_BLACK, HAIR_WHITE, HAIR_BROWN, HAIR_BLONDE, HAIR_RED, };
UENUM(BlueprintType)
enum class EBuildTrait : uint8 { BLD_SLIM, BLD_THIN, BLD_AVERAGE, BLD_ATHLETIC, BLD_MUSCULAR, BLD_STOCKY, BLD_HUSKY, BLD_HEAVYSET, };
UENUM(BlueprintType)
enum class EDecisionStyle : uint8 { DEC_XTR_IMPULSIVE, DEC_MLD_IMPULSIVE, DEC_MLD_CALCULATING, DEC_XTR_CALCULATING, };
UENUM(BlueprintType)
enum class EConflictStyle : uint8 { CNF_XTR_CONFRONT, DEC_MLD_CONFRONT, DEC_MLD_SECRET, DEC_XTR_SECRET, };

/// <summary>
/// Represents on person of the case
/// </summary>
USTRUCT(BlueprintType)
struct FCasePerson {
	GENERATED_BODY()

	UPROPERTY()
	int32 id;

	UPROPERTY()
	FString name;

	UPROPERTY()
	FString alias;

	UPROPERTY()
	int32 age;

	UPROPERTY()
	ESex sex;

	UPROPERTY()
	EEyeColor eye_color;

	UPROPERTY()
	EHairColor hair_color;

	UPROPERTY()
	EBuildTrait physical_build;

	UPROPERTY()
	int32 loc_residence;

	UPROPERTY()
	FString phone;

	UPROPERTY()
	FString email;

	UPROPERTY()
	FString occupation;

	UPROPERTY()
	EDecisionStyle decision_personality;

	UPROPERTY()
	EConflictStyle conflict_personality;
};
// ----------------------------------------------------------------------------

/// <summary>
/// All possible matrix attributes for locations in the case
/// </summary>
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
};

UENUM(BlueprintType)
enum class ELocationType : uint8 { LOCTYPE_RESIDENCE, LOCTYPE_COMMERCIAL, LOCTYPE_OUTDOOR, LOCTYPE_TRANSPORT, LOCTYPE_INDUSTRIAL, };
UENUM(BlueprintType)
enum class EAccessLevel : uint8 { ACCESS_PUBLIC, ACCESS_SEMIPUBLIC, ACCESS_PRIVATE, ACCESS_RESTRICTED, };
UENUM(BlueprintType)
enum class ESecurityLevel : uint8 { SEC_NONE, SEC_LOW, SEC_HIGH, };
UENUM(BlueprintType)
enum class ELocationCapacity : uint8 { CAP_LOW, CAP_MEDIUM, CAP_HIGH, };

/// <summary>
/// Represents one location of the case
/// </summary>
USTRUCT(BlueprintType)
struct FCaseLocation {
	GENERATED_BODY()

	UPROPERTY()
	int32 id;

	UPROPERTY()
	ELocationType type;

	UPROPERTY()
	FString name_address;

	UPROPERTY()
	int32 owner;

	UPROPERTY()
	TArray<int32> residents;

	UPROPERTY()
	EAccessLevel access_level;

	UPROPERTY()
	ESecurityLevel security;

	UPROPERTY()
	FString op_hours;

	UPROPERTY()
	ELocationCapacity capacity;

	UPROPERTY()
	TArray<int32> connected_to;
};
// ----------------------------------------------------------------------------

/// <summary>
/// All possible matrix attributes for events in the case
/// </summary>
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
enum class EEventType : uint8 {
	ETYPE_MEETING,
	ETYPE_ARGUMENT,
	ETYPE_THREAT,

	ETYPE_MOVEMENT,
	ETYPE_OBJ_ACQUISITION,
	ETYPE_TRANSACTION,

	ETYPE_COMM_CALL,
	ETYPE_COMM_TEXT,
	ETYPE_COMM_WRITTEN,

	ETYPE_VIOLENCE,
	ETYPE_COMMIT,

	ETYPE_OBJ_DISPOSAL,
	ETYPE_DISCOVERY,
};
UENUM(BlueprintType)
enum class EEventVisibility : uint8 { VIS_PUBLIC, VIS_SEMIPUBLIC, VIS_PRIVATE, VIS_SECRET, };

/// <summary>
/// Represents one event of the case
/// </summary>
USTRUCT(BlueprintType)
struct FCaseEvent {
	GENERATED_BODY()

	UPROPERTY()
	int32 id;

	UPROPERTY()
	EEventType type;

	UPROPERTY()
	FDateTime date_start;

	UPROPERTY()
	FDateTime date_end;

	UPROPERTY()
	int32 location;

	UPROPERTY()
	TArray<int32> participants;

	UPROPERTY()
	TArray<int32> parents;

	UPROPERTY()
	EEventVisibility visibility;
};
// ----------------------------------------------------------------------------

/// <summary>
/// All possible matrix attributes for relations in the case
/// </summary>
UENUM(BlueprintType)
enum class ERelationInfoType : uint8 {
	REL_ID,
	REL_TYPE,
	REL_PERSONA,
	REL_PERSONB,
    REL_VALENCE,
    REL_INTENSITY,
    REL_PUBLICITY,
    REL_STARTEDAT,
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

UENUM(BlueprintType)
enum class ERelationValence : uint8 { VAL_POSITIVE, VAL_NEUTRAL, VAL_NEGATIVE, VAL_COMPLEX, };
UENUM(BlueprintType)
enum class ERelationIntensity : uint8 { INT_LOW, INT_MEDIUM, INT_HIGH, };
UENUM(BlueprintType)
enum class ERelationPublicity : uint8 { PUB_PUBLIC, PUB_PRIVATE, PUB_SECRET, };

/// <summary>
/// Represents one relation between two persons of the case
/// </summary>
USTRUCT(BlueprintType)
struct FCaseRelation {
	GENERATED_BODY()

	UPROPERTY()
	int32 id;

	UPROPERTY()
	ERelationType type;

	UPROPERTY()
	int32 person_a;

	UPROPERTY()
	int32 person_b;

	UPROPERTY()
	ERelationValence valence;

	UPROPERTY()
	ERelationIntensity intensity;

	UPROPERTY()
	ERelationPublicity publicity;

	UPROPERTY()
	FDateTime started_at;
};
// ----------------------------------------------------------------------------
