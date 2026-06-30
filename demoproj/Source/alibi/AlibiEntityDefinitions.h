// Fill out your copyright notice in the Description page of Project Settings.

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

/// <summary>
/// Represents one location of the case
/// </summary>
USTRUCT(BlueprintType)
struct FCaseLocation {
	GENERATED_BODY()

	UPROPERTY()
	FName id;

	UPROPERTY()
	FName name;

	UPROPERTY()
	ELocationType type;

	UPROPERTY()
	FText desc;

	UPROPERTY()
	TArray<FName> links;

	UPROPERTY()
	FName ink;
};
// ----------------------------------------------------------------------------

/// <summary>
/// Represents one action the player can choose
/// </summary>
USTRUCT()
struct FCaseAction {
	GENERATED_BODY()

	UPROPERTY()
	FName id;

	UPROPERTY()
	FName loc_id;

	UPROPERTY()
	FText label;

	UPROPERTY()
	FName ink_knot;

	UPROPERTY()
	FText reveals;

	UPROPERTY()
	TArray<FName> require;

	UPROPERTY()
	TArray<FName> grants;

	UPROPERTY()
	bool hidden;
};
// ----------------------------------------------------------------------------
