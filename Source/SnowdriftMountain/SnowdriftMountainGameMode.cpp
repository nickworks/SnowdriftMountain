// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowdriftMountainGameMode.h"
#include "SnowdriftMountainCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASnowdriftMountainGameMode::ASnowdriftMountainGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Snowboarder"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ASnowdriftMountainGameMode::StartPlay()
{
	Super::StartPlay();
	//GetWorld()->GetFirstPlayerController()->Possess();
}
