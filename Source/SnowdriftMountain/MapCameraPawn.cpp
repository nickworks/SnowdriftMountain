// Fill out your copyright notice in the Description page of Project Settings.


#include "MapCameraPawn.h"

AMapCameraPawn::AMapCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AMapCameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMapCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMapCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

