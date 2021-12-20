// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EBoarderState : uint8
{
	InAir				UMETA(DisplayName = "In the air"),
	OnGround			UMETA(DisplayName = "Grounded"),
	OnGroundCouching	UMETA(DisplayName = "Grounded crouching"),
	InAirCrouching		UMETA(DisplayName = "In-air crouching"),
	Ragdoll				UMETA(DisplayName = "Ragoll")
};
