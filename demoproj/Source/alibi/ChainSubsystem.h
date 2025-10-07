// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ChainSubsystem.generated.h"

/**
 * Subsystem that will track the chain of events that happened during the case being investigated
 * The chain will be initialized with the full details at the start of the case and some elements
 * will be hidden from the player. These removed elements need to be filled in by the player from the Matrix
 */
UCLASS()
class ALIBI_API UChainSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
};
