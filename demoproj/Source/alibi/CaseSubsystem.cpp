// Fill out your copyright notice in the Description page of Project Settings.

#include "CaseSubsystem.h"

DEFINE_LOG_CATEGORY(LogCase);

#define CASE_HEADER_ID "[case]"
#define LOCATION_HEADER_ID "[location]"
#define ACTION_HEADER_ID "[action]"

void parse_metadata(FCaseData &c, const TArray<FString> &lines, size_t &offset) {
	while (!lines[offset].Contains(CASE_HEADER_ID)) {
		offset++;
	}
	offset++;

	while (offset < lines.Num() && !lines[offset].IsEmpty()) {
		if (lines[offset][0] == '#') {
			offset++;
			continue;
		}

		FString key;
		FString val;
		if (!lines[offset].Split("=", &key, &val)) {
			// log error
			offset++;
			continue;
		}
		key.TrimStartAndEndInline();
		val.TrimStartAndEndInline();

		UE_LOG(LogCase, Log, TEXT("[CASE PARSE] - {'%s' ; '%s'}"), *key, *val);
		if (key == "name") {
			c.name = FText::FromString(val);
		}
		else if (key == "desc") {
			c.desc = FText::FromString(val);
		}
		else if (key == "intro") {
			c.intro_knot = *val;
		}
		else if (key == "version") {
			int32 ver = 0;
			LexFromString(ver, val);
			c.version = ver;
		}

		offset++;
	}
}

void parse_location(TArray<FCaseLocation> &ls, TArray<FString> &lines, size_t &offset) {
	offset++; // skip header line
	FCaseLocation l{};

	while (offset < lines.Num() && !lines[offset].IsEmpty()) {
		if (lines[offset][0] == '#') {
			offset++;
			continue;
		}

		FString key;
		FString val;
		if (!lines[offset].Split("=", &key, &val)) {
			// log error
			offset++;
			continue;
		}
		key.TrimStartAndEndInline();
		val.TrimStartAndEndInline();

		UE_LOG(LogCase, Log, TEXT("[LOCATION PARSE] - {'%s' ; '%s'}"), *key, *val);
		if (key == "id") {
			l.id = *val;
		}
		else if (key == "name") {
			l.name = *val;
		}
		else if (key == "type") {
			if (val == "RESIDENCE") l.type = ELocationType::LOCTYPE_RESIDENCE;
			else if (val == "COMMERCIAL") l.type = ELocationType::LOCTYPE_COMMERCIAL;
			else if (val == "OUTDOOR") l.type = ELocationType::LOCTYPE_OUTDOOR;
			else if (val == "TRANSPORT") l.type = ELocationType::LOCTYPE_TRANSPORT;
			else if (val == "INDUSTRIAL") l.type = ELocationType::LOCTYPE_INDUSTRIAL;
		}
		else if (key == "desc") {
			l.desc = FText::FromString(val);
		}
		else if (key == "links") {
			TArray<FString> link_names;
			val.ParseIntoArray(link_names, TEXT(", "));

			for (auto& link : link_names) {
				l.links.Add(*link);
			}
		}
		else if (key == "ink") {
			l.ink = *val;
		}

		offset++;
	}

	ls.Add(l);
}

void parse_action(TArray<FCaseAction> &as, TArray<FString> &lines, size_t &offset) {
	offset++; // skip header line
	FCaseAction a{};

	while (offset < lines.Num() && !lines[offset].IsEmpty()) {
		if (lines[offset][0] == '#') {
			offset++;
			continue;
		}

		FString key;
		FString val;
		if (!lines[offset].Split("=", &key, &val)) {
			// log error
			offset++;
			continue;
		}
		key.TrimStartAndEndInline();
		val.TrimStartAndEndInline();

		UE_LOG(LogCase, Log, TEXT("[ACTION PARSE] - {'%s' ; '%s'}"), *key, *val);
		if (key == "id") {
			a.id = *val;
		}
		else if (key == "location") {
			a.loc_id = *val;
		}
		else if (key == "label") {
			a.label = FText::FromString(val);
		}
		else if (key == "ink") {
			a.ink_knot = *val;
		}
		else if (key == "reveal") {
			a.reveals = FText::FromString(val);
		}
		else if (key == "requires") {
			TArray<FString> req_names;
			val.ParseIntoArray(req_names, TEXT(", "));

			for (auto& req : req_names) {
				a.require.Add(*req);
			}
		}
		else if (key == "grants") {
			TArray<FString> grant_names;
			val.ParseIntoArray(grant_names, TEXT(", "));

			for (auto& grant : grant_names) {
				a.grants.Add(*grant);
			}
		}
		else if (key == "hidden") {
			LexFromString(a.hidden, val);
		}

		offset++;
	}

	as.Add(a);
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
	for (; position < file.Num(); position++) {
		if (file[position].Contains(LOCATION_HEADER_ID)) {
			parse_location(locations, file, position);
		}
		else if (file[position].Contains(ACTION_HEADER_ID)) {
			parse_action(actions, file, position);
		}
	}
}
