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

	UE_LOG(LogCase, Log, TEXT("load_case_file: loaded '%s' — case '%s' (%d facts, %d actions, %d locations)."),
		*case_path, *meta.case_id.ToString(), facts.Num(), actions.Num(), locations.Num());
}
