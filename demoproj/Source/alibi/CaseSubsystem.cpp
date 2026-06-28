// Fill out your copyright notice in the Description page of Project Settings.

#include "CaseSubsystem.h"

void parse_metadata(TArray<FString> lines) {
}
void parse_location(TArray<FString> lines) {
}
void parse_action(TArray<FString> lines) {
}

void UCaseSubsystem::load_case_file(FString case_path) {
	current_case.name = FText::FromString("Case #1");
	current_case.desc = FText::FromString("Case - Lorem Ipsum");
	current_case.intro_knot = TEXT("nil");
	current_case.version = 0;
	//TODO: Parse file at case_path here
}
