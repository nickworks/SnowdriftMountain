// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SnowboarderMovementComponent.generated.h"

UENUM(BlueprintType)
enum class ECustomMovementType : uint8
{
	None		= 0 UMETA(DisplayName = "None"),
	Boarding	= 1 UMETA(DisplayName = "Boarding")
};


/**
 * 
 */
UCLASS()
class SNOWDRIFTMOUNTAIN_API USnowboarderMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
		
protected:
		USnowboarderMovementComponent(const FObjectInitializer& ObjectInitializer);
		void PhysCustom(float deltaTime, int32 Iterations) override;
		void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

		UFUNCTION()
		void PhysBoard(float deltaTime, int32 Iterations);

};
