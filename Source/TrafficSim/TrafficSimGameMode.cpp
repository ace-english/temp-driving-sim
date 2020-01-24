// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "TrafficSimGameMode.h"
#include "TrafficSimPawn.h"
#include "TrafficSimHud.h"

ATrafficSimGameMode::ATrafficSimGameMode()
{
	DefaultPawnClass = ATrafficSimPawn::StaticClass();
	HUDClass = ATrafficSimHud::StaticClass();
}

UTimeManager* ATrafficSimGameMode::GetTimeManager() {
	if (simTimeManager == nullptr) {
		simTimeManager = NewObject<UTimeManager>(GetTransientPackage(), UTimeManager::StaticClass());
	}

	return simTimeManager;
}