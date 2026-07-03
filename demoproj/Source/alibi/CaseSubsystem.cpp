// Fill out your copyright notice in the Description page of Project Settings.

#include "CaseSubsystem.h"
#include "JsonObjectConverter.h"

DEFINE_LOG_CATEGORY(LogCase);

void UCaseSubsystem::load_case_file(FString case_path) {
	FString contents;
	if (!FFileHelper::LoadFileToString(contents, *case_path)) {
		// loggin error here
		return;
	}

	memset(&current_case, 0, sizeof(current_case));
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(contents, &current_case)) {
		// loggin error here
		return;
	}
}
