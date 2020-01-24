// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "Environment/TimeManager.h"
#include "TrafficSimGameMode.generated.h"

//TODO for maintenence manual - make note that GameMode, while present in all Unreal Projects, is only used on the server of multiplayer games and is not actually used at all for the simulation.
UCLASS(minimalapi)
class ATrafficSimGameMode : public AGameModeBase
{
	GENERATED_BODY()
protected:
	UTimeManager* simTimeManager = nullptr;

public:
	ATrafficSimGameMode();

	UTimeManager* GetTimeManager();
};



