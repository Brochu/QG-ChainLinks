// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AlibiPlayerController.generated.h"

/**
 * base Player Controller class for Alibi Project
 */
UCLASS()
class ALIBI_API AAlibiPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AAlibiPlayerController();

	virtual void Tick(float dt) override;

	UFUNCTION(BlueprintCallable)
	FVector2f CalcCameraMoveMults();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alibi")
	FVector2f ActiveZoneRelSize = { 0.1f, 0.1f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alibi")
	FVector2f CameraRotationOffsets = { 1.f, 1.f };

protected:
	virtual void BeginPlay() override;
};
