// Fill out your copyright notice in the Description page of Project Settings.

#include "AlibiGameModeBase.h"
#include "AlibiGameStateBase.h"
#include "AlibiPlayerController.h"
#include "AlibiHUDBase.h"

AAlibiGameModeBase::AAlibiGameModeBase() {
	this->GameStateClass = AAlibiGameStateBase::StaticClass();
	this->PlayerControllerClass = AAlibiPlayerController::StaticClass();
	this->HUDClass = AAlibiHUDBase::StaticClass();
	this->DefaultPawnClass = nullptr;
}
