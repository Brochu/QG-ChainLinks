// Fill out your copyright notice in the Description page of Project Settings.

#include "AlibiPlayerController.h"
#include "Framework/Application/SlateApplication.h"

AAlibiPlayerController::AAlibiPlayerController() {
	this->SetShowMouseCursor(true);
}

void AAlibiPlayerController::BeginPlay() {
	Super::BeginPlay();
}

void AAlibiPlayerController::Tick(float dt) {
	Super::Tick(dt);
}

FVector2f AAlibiPlayerController::CalcCameraMoveMults() {
	int32 vp_x, vp_y;
	GetViewportSize(vp_x, vp_y);
	float dead_x0 = vp_x * ActiveZoneRelSize.X;
	float dead_x1 = vp_x - dead_x0;
	UE_LOG(LogTemp, Log, TEXT("Deadzone X0 = %f"), dead_x0);

	float dead_y0 = vp_y * ActiveZoneRelSize.Y;
	float dead_y1 = vp_y - dead_y0;

	float m_x, m_y;
	GetMousePosition(m_x, m_y);

	FVector2f res = { 0.f, 0.f };
	if (m_x < dead_x0) {
		res.X = -1.f;
	}
	else if (m_x > dead_x1) {
		res.X = 1.f;
	}

	if (m_y < dead_y0) {
		res.Y = -1.f;
	}
	else if (m_y > dead_y1) {
		res.Y = 1.f;
	}

	return res;
}
