// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnowdriftMountainGameMode.generated.h"

UCLASS(minimalapi)
class ASnowdriftMountainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASnowdriftMountainGameMode();
	
	virtual void StartPlay() override;
};



