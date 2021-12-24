// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MapCameraPawn.generated.h"

UCLASS()
class SNOWDRIFTMOUNTAIN_API AMapCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AMapCameraPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
