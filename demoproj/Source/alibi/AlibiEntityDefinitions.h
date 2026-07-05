// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AlibiEntityDefinitions.generated.h"

// NOTE: the case-file `FCaseLocation` / `FCaseAction` structs now live in
// CaseSubsystem.h (Case File Data Inventory v2.0). The old matrix-era versions
// that used to sit here were removed to avoid a redefinition clash.
// ----------------------------------------------------------------------------

// ============================================================================
//  COLD FILE case-logic schema (Case File Data Inventory v2.0).
//  These structs mirror the JSON 1:1 so a case file deserializes straight into
//  FCaseFile (e.g. via FJsonObjectConverter::JsonObjectStringToUStruct).
//
//  Parsing notes:
//   - Field names match the JSON keys exactly (snake_case). FJsonObjectConverter
//     matches property names case-insensitively.
//   - The small closed-vocabulary strings are UENUMs; string import matches the
//     enumerator name (which is spelled exactly as the JSON value).
//   - JSON keys that may be `null` or absent (prerequisites, unlock_rule, when,
//     blocks_of_day, location_states, conditions, alternates) map to TArrays that
//     simply come back empty.
//   - Two spots need a hand in the loader (called out at their fields):
//     reconstruction `template` (C++ keyword) and slot `decoy_pool` (string|array).
// ============================================================================

// ---- closed vocabularies ---------------------------------------------------
UENUM()
enum class EGlossaryKind : uint8 { PERSON, PLACE, OBJECT, VEHICLE, ORG, TIME };

UENUM()
enum class ECaseReliability : uint8 { TESTIMONY, DOCUMENT, FORENSIC };

UENUM()
enum class ECaseVerb : uint8 { INSPECT, SEARCH, COLLECT, INTERVIEW, CANVASS, PHONE_FAX };

UENUM()
enum class EReconBand : uint8 { BEFORE, DURING, AFTER };

UENUM()
enum class EReconSlotKind : uint8 { ACTOR, VERB, OBJECT, PLACE, TIME, MOTIVE };

// ---- meta ------------------------------------------------------------------
USTRUCT()
struct FCaseSetting {
	GENERATED_BODY()

	UPROPERTY()
	FText date;

	UPROPERTY()
	FText region;
};

USTRUCT()
struct FCaseMetadata {
	GENERATED_BODY()

	UPROPERTY()
	FName case_id;

	UPROPERTY()
	FText title;

	UPROPERTY()
	FCaseSetting setting;

	UPROPERTY()
	int32 deadline_days = 0;

	UPROPERTY()
	int32 blocks_per_day = 0;

	// The single ink file for this case; every knot reference resolves inside it.
	UPROPERTY()
	FName ink_file;

	UPROPERTY()
	FName briefing_knot;

	UPROPERTY()
	TArray<FName> starting_facts;

	UPROPERTY()
	TArray<FName> starting_locations;

	UPROPERTY()
	int32 lab_queue_capacity = 0;
};

// ---- glossary --------------------------------------------------------------
USTRUCT()
struct FCaseGlossaryEntry {
	GENERATED_BODY()

	UPROPERTY()
	FName tag_id;

	UPROPERTY()
	EGlossaryKind kind = EGlossaryKind::OBJECT;

	UPROPERTY()
	FText display_name;
};

// ---- facts -----------------------------------------------------------------
USTRUCT()
struct FCaseFact {
	GENERATED_BODY()

	UPROPERTY()
	FName fact_id;

	UPROPERTY()
	FText text;

	UPROPERTY()
	TArray<FName> tags;

	UPROPERTY()
	ECaseReliability reliability = ECaseReliability::TESTIMONY;

	// TESTIMONY-only observation metadata; empty otherwise.
	UPROPERTY()
	TArray<FText> conditions;
};

// ---- contradictions --------------------------------------------------------
USTRUCT()
struct FCaseContradiction {
	GENERATED_BODY()

	UPROPERTY()
	FName contradiction_id;

	// Exactly two, symmetric.
	UPROPERTY()
	TArray<FName> facts;

	UPROPERTY()
	TArray<FName> resolution_actions;
};

// ---- locations -------------------------------------------------------------
USTRUCT()
struct FCaseLocationState {
	GENERATED_BODY()

	UPROPERTY()
	FName state_id;

	// Fact list (AND) that switches the location into this state; empty (JSON null)
	// on the initial state. Later matching states win over earlier ones.
	UPROPERTY()
	TArray<FName> when;
};

USTRUCT(BlueprintType)
struct FCaseLocation {
	GENERATED_BODY()

	UPROPERTY()
	FName location_id;

	UPROPERTY()
	FText name;

	// Fact list (AND); empty (JSON null) means reachable only via starting_locations.
	UPROPERTY()
	TArray<FName> unlock_rule;

	// Ordered; states[0] is the initial state.
	UPROPERTY()
	TArray<FCaseLocationState> states;
};

// ---- actions (every interactable thing, INSPECT included) -------------------
USTRUCT(BlueprintType)
struct FCaseAction {
	GENERATED_BODY()

	UPROPERTY()
	FName action_id;

	UPROPERTY()
	ECaseVerb verb = ECaseVerb::INSPECT;

	UPROPERTY()
	FName location_id;

	UPROPERTY()
	FText label;

	UPROPERTY()
	int32 cost = 0;

	// Fact list (AND); empty (JSON null) = no prerequisite.
	UPROPERTY()
	TArray<FName> prerequisites;

	UPROPERTY()
	bool hidden = false;

	UPROPERTY()
	FText locked_hint;

	// Absolute block range [from, to] inclusive; to == -1 means "end of case".
	UPROPERTY()
	TArray<int32> available;

	// Optional periodic filter, e.g. [2] = evenings only; empty = every block.
	UPROPERTY()
	TArray<int32> blocks_of_day;

	// Diorama states this action exists in; empty = all states.
	UPROPERTY()
	TArray<FName> location_states;

	// Angle-only discovery (staging holds the angle). Never the sole route to a
	// critical fact (validator law).
	UPROPERTY()
	bool hidden_reveal = false;

	// Blocks spent before results mature; 0 = immediate. COLLECT additionally
	// occupies a lab-queue slot for the delay (engine rule from the verb).
	UPROPERTY()
	int32 delay = 0;

	// Required whenever delay > 0: in-fiction delay line + pager headline.
	UPROPERTY()
	FText pending_label;

	// Flat, unconditional fact list; empty for INTERVIEW (its manifest owns output).
	UPROPERTY()
	TArray<FName> produces;

	UPROPERTY()
	bool repeatable = false;
};

// ---- interviews ------------------------------------------------------------
USTRUCT()
struct FCaseInterview {
	GENERATED_BODY()

	UPROPERTY()
	FName interview_id;

	// The INTERVIEW-verb action this payload belongs to (single direction).
	UPROPERTY()
	FName action_id;

	UPROPERTY()
	FName character;

	// Knot inside meta.ink_file.
	UPROPERTY()
	FName knot;

	// Every fact the knot can discoverFact(); validator cross-checks the ink source.
	UPROPERTY()
	TArray<FName> fact_manifest;

	UPROPERTY()
	bool re_interview = false;
};

// ---- schedule (timed unprompted delivery) ----------------------------------
USTRUCT()
struct FCaseScheduleEntry {
	GENERATED_BODY()

	// Absolute block index = day * blocks_per_day + block (0-based).
	UPROPERTY()
	int32 at_block = 0;

	UPROPERTY()
	TArray<FName> delivers;

	UPROPERTY()
	FText pager;
};

// ---- reconstruction --------------------------------------------------------
USTRUCT()
struct FReconPhrase {
	GENERATED_BODY()

	UPROPERTY()
	FName id;

	UPROPERTY()
	FText text;
};

USTRUCT()
struct FReconAlternate {
	GENERATED_BODY()

	UPROPERTY()
	FName id;

	// Partial-credit weight in (0,1).
	UPROPERTY()
	float weight = 0.f;
};

USTRUCT()
struct FReconSlot {
	GENERATED_BODY()

	UPROPERTY()
	EReconSlotKind kind = EReconSlotKind::ACTOR;

	// Glossary ID (entity kinds) or phrase ID (VERB / MOTIVE).
	UPROPERTY()
	FName answer;

	UPROPERTY()
	TArray<FReconAlternate> alternates;

	// JSON `decoy_pool` is EITHER a pool token ("discovered_persons", ...) OR an
	// explicit id list. FJsonObjectConverter can't target one field with both
	// shapes, so the loader splits it: set `decoy_pool_token` when the JSON value
	// is a string, otherwise fill `decoy_pool_ids`. Exactly one is populated.
	UPROPERTY()
	FName decoy_pool_token;

	UPROPERTY()
	TArray<FName> decoy_pool_ids;

	UPROPERTY()
	TArray<FName> supports;
};

USTRUCT()
struct FReconEvent {
	GENERATED_BODY()

	UPROPERTY()
	FName event_id;

	UPROPERTY()
	EReconBand band = EReconBand::BEFORE;

	// JSON key is "template" (a C++ keyword). FJsonObjectConverter matches names
	// case-insensitively, so "template" -> Template; a hand-rolled parser must map
	// it explicitly.
	UPROPERTY()
	FText Template;

	UPROPERTY()
	TArray<FReconSlot> slots;
};

USTRUCT()
struct FReconAssertionWeights {
	GENERATED_BODY()

	UPROPERTY()
	float who = 0.f;

	UPROPERTY()
	float how = 0.f;

	UPROPERTY()
	float why = 0.f;

	UPROPERTY()
	float before_after = 0.f;

	UPROPERTY()
	float accessory = 0.f;
};

USTRUCT()
struct FReconEventBandMap {
	GENERATED_BODY()

	UPROPERTY()
	TArray<FName> who;

	UPROPERTY()
	TArray<FName> how;

	UPROPERTY()
	TArray<FName> why;

	UPROPERTY()
	TArray<FName> before_after;

	UPROPERTY()
	TArray<FName> accessory;
};

USTRUCT()
struct FReconGrading {
	GENERATED_BODY()

	UPROPERTY()
	FReconAssertionWeights assertion_weights;

	UPROPERTY()
	FReconEventBandMap event_band_map;
};

USTRUCT()
struct FCaseReconstruction {
	GENERATED_BODY()

	UPROPERTY()
	TArray<FReconPhrase> verb_list;

	UPROPERTY()
	TArray<FReconPhrase> motive_list;

	UPROPERTY()
	TArray<FReconEvent> events;

	UPROPERTY()
	FReconGrading grading;
};

// ---- outcome tiers (single source for score thresholds) --------------------
USTRUCT()
struct FOutcomeTier {
	GENERATED_BODY()

	UPROPERTY()
	FName tier;

	// Minimum score for this tier; list is ordered descending.
	UPROPERTY()
	float min = 0.f;

	// Outcome-memo knot inside meta.ink_file.
	UPROPERTY()
	FName knot;
};

// ---- root ------------------------------------------------------------------
USTRUCT()
struct FCaseFile {
	GENERATED_BODY()

	UPROPERTY()
	FString schema_version;

	UPROPERTY()
	FCaseMetadata meta;

	UPROPERTY()
	TArray<FCaseGlossaryEntry> glossary;

	UPROPERTY()
	TArray<FCaseFact> facts;

	UPROPERTY()
	TArray<FCaseContradiction> contradictions;

	UPROPERTY()
	TArray<FCaseLocation> locations;

	UPROPERTY()
	TArray<FCaseAction> actions;

	UPROPERTY()
	TArray<FCaseInterview> interviews;

	UPROPERTY()
	TArray<FCaseScheduleEntry> schedule;

	UPROPERTY()
	FCaseReconstruction reconstruction;

	UPROPERTY()
	TArray<FOutcomeTier> outcome_tiers;
};
