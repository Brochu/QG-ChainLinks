// Fill out your copyright notice in the Description page of Project Settings.

#include "CaseSubsystem.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogCase);

void UCaseSubsystem::load_case_file(FString case_path) {
	FString contents;
	if (!FFileHelper::LoadFileToString(contents, *case_path)) {
		UE_LOG(LogCase, Error, TEXT("load_case_file: could not read case file '%s' (file missing or unreadable)."), *case_path);
		return;
	}

	TSharedPtr<FJsonObject> root;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(contents);
	if (!FJsonSerializer::Deserialize(reader, root) || !root.IsValid()) {
		UE_LOG(LogCase, Error,
			TEXT("load_case_file: malformed JSON in '%s': %s"),
			*case_path, *reader->GetErrorMessage());
		return;
	}

	FCaseFile case_data;
	if (!FJsonObjectConverter::JsonObjectToUStruct(root.ToSharedRef(), &case_data)) {
		UE_LOG(LogCase, Error, TEXT("load_case_file: '%s' is valid JSON but does not match the case schema (field type mismatch?)."), *case_path);
		return;
	}

	meta = case_data.meta;
	glossary = case_data.glossary;
	facts = case_data.facts;
	contradictions = case_data.contradictions;
	locations = case_data.locations;
	actions = case_data.actions;
	interviews = case_data.interviews;
	schedule = case_data.schedule;
	reconstruction = case_data.reconstruction;
	outcome_tiers = case_data.outcome_tiers;

	if (locations.Num() == 0) {
		UE_LOG(LogCase, Error, TEXT("load_case_file: '%s' parsed but contains no locations; cannot start the case."), *case_path);
		return;
	}

	// Always start at first location in the case file
	active_locid = locations[0].location_id;
	used_blocks = 0;
	known_facts.Empty();
	active_tags.Empty();
	active_lab_requests.Empty();

	UE_LOG(LogCase, Log, TEXT("load_case_file: loaded '%s' — case '%s' (%d facts, %d actions, %d locations)."),
		*case_path,
		*meta.case_id.ToString(),
		facts.Num(),
		actions.Num(),
		locations.Num()
	);
}

FCaseLocation &UCaseSubsystem::get_active_location() {
	FCaseLocation *active_loc = locations.FindByPredicate([this](const FCaseLocation &other) { return other.location_id == active_locid; });

	if (active_loc == nullptr) {
		// Invariant break: active_locid should always name a loaded location.
		UE_LOG(LogCase, Error,
			TEXT("get_active_location: active_locid '%s' is not a loaded location; falling back to the first."),
			*active_locid.ToString());
		active_loc = &locations[0];
	}
	return *active_loc;
}

TArray<int32> UCaseSubsystem::list_location_idx() {
	TArray<int32> results;

	for (int32 i = 0; i < locations.Num(); i++) {
		results.Add(i);
	}

	return results;
}

TArray<int32> UCaseSubsystem::list_action_idx(FName loc_id) {
	TArray<int32> results;

	for (int32 i = 0; i < actions.Num(); i++) {
		if (actions[i].location_id != loc_id) {
			continue;
		}
		results.Add(i);
	}

	return results;
}

bool UCaseSubsystem::move_location(FName new_loc_id) {
	//TODO: Overly simple for now, will need events and checks later to react to movement during a case
	active_locid = new_loc_id;
	return true;
}

bool UCaseSubsystem::commit_action(FName action_id) {
	FCaseAction *chosen_action = actions.FindByPredicate([action_id](const FCaseAction &other) { return other.action_id == action_id; });
	if (chosen_action == nullptr) {
		return false;
	}

	if ((used_blocks + chosen_action->cost) > (meta.deadline_days * meta.blocks_per_day)) {
		return false;
	}

	if (chosen_action->delay > 0) {
		// Lab request action; delayed results
		if (active_lab_requests.Num() >= meta.lab_queue_capacity) {
			return false;
		}
		active_lab_requests.Push({ action_id, used_blocks });
		on_new_lab_request.Broadcast(active_lab_requests.Last());
	}

	spend_blocks(chosen_action->cost);
	return true;
}

void UCaseSubsystem::spend_blocks(int32 quantity) {
	int32 prev_blocks = used_blocks;
	used_blocks += quantity;

	//TODO: Handle time moving forward
	// Handle lab requests ending
	// Handle schedule entries

	on_block_spent.Broadcast(prev_blocks, used_blocks);
}
