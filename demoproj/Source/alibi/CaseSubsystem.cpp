// Fill out your copyright notice in the Description page of Project Settings.

#include "CaseSubsystem.h"

DEFINE_LOG_CATEGORY(LogCase);

void parse_metadata(FCaseData &c, const TArray<FString> &lines, size_t &offset) {
	while (!lines[offset].Contains("[case]")) {
		offset++;
	}
	offset++;

	while (!lines[offset].IsEmpty()) {
		FString key;
		FString val;
		if (lines[offset].Split("=", &key, &val)) {
			// log error
		}
		key.TrimStartAndEndInline();
		val.TrimStartAndEndInline();

		UE_LOG(LogCase, Log, TEXT("[CASE PARSE] - {'%s' ; '%s'}"), *key, *val);
		offset++;
	}
}

void parse_location(FCaseData &c, TArray<FString> &lines, size_t &offset) {
}
void parse_action(FCaseData &c, TArray<FString> &lines, size_t &offset) {
}

void UCaseSubsystem::load_case_file(FString case_path) {
	TArray<FString> file;
	if (!FFileHelper::LoadFileToStringArray(file, *case_path)) {
		// loggin error here
		return;
	}

	current_case.name = FText::FromString("");
	current_case.desc = FText::FromString("");
	current_case.intro_knot = "";
	current_case.version = 0;
	size_t position = 0;
	parse_metadata(current_case, file, position);

	//TODO: Parse file at case_path here
}
