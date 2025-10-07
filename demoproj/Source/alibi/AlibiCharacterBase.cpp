// Fill out your copyright notice in the Description page of Project Settings.


#include "AlibiCharacterBase.h"

// Sets default values
AAlibiCharacterBase::AAlibiCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAlibiCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAlibiCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAlibiCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

