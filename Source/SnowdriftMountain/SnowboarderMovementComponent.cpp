// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/Character.h"
#include "SnowboarderMovementComponent.h"

USnowboarderMovementComponent::USnowboarderMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {

	if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 3., FColor::Red, "startup move component");
}
void USnowboarderMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
    Super::PhysCustom(deltaTime, Iterations);


    switch (CustomMovementMode)
    {
        case static_cast<int>(ECustomMovementType::Boarding) :
        default:
	        //GEngine->AddOnScreenDebugMessage(-1, 3., FColor::Red, "process board physics!");
            PhysBoard(deltaTime, Iterations);
            break;
    }
}
void USnowboarderMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	FString wasMode = StaticEnum<EMovementMode>()->GetNameStringByValue(static_cast<int32>(PreviousMovementMode));
	FString iisMode = StaticEnum<EMovementMode>()->GetNameStringByValue(static_cast<int32>(MovementMode));

    //GEngine->AddOnScreenDebugMessage(-1, 3., FColor::Cyan, "Move mode changed from " + wasMode + " to " + iisMode);
}


void USnowboarderMovementComponent::PhysBoard(float deltaTime, int32 Iterations)
{
    
    if (deltaTime < MIN_TICK_TIME)
    {
        return;
    }

    if (!CurrentFloor.IsWalkableFloor()) {

    }

    RestorePreAdditiveRootMotionVelocity();

    if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        if (bCheatFlying && Acceleration.IsZero())
        {
            Velocity = FVector::ZeroVector;
        }
        const float Friction = GroundFriction;//0.5f * GetPhysicsVolume()->FluidFriction;
        CalcVelocity(deltaTime, Friction, true, GetMaxBrakingDeceleration());
    }

    ApplyRootMotionToVelocity(deltaTime);

    Iterations++;
    bJustTeleported = false;

    FVector OldLocation = UpdatedComponent->GetComponentLocation();
    const FVector MoveVelocity = Velocity;
    const FVector Delta = deltaTime * MoveVelocity;
    const bool bZeroDelta = Delta.IsNearlyZero();

    FStepDownResult stepDown;

    // Attempt to move:
    MoveAlongFloor(Velocity, deltaTime, &stepDown);


    if (IsFalling())
    {
        // pawn decided to jump up
        /**
        const float DesiredDist = Delta.Size();
        if (DesiredDist > KINDA_SMALL_NUMBER)
        {
            const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
            remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
        }
        StartNewPhysics(remainingTime, Iterations);
        /**/
        return;
    }

    // Update floor.
    // StepUp might have already done it for us.
    if (stepDown.bComputedFloor)
    {
        CurrentFloor = stepDown.FloorResult;
    }
    else
    {
        FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
    }


    if (!CurrentFloor.IsWalkableFloor())// && !CurrentFloor.HitResult.bStartPenetrating)
    {

        GEngine->AddOnScreenDebugMessage(-1, 3., FColor::Cyan, "bad floor, start falling?");
        SetMovementMode(MOVE_Falling);
    }

    MaintainHorizontalGroundVelocity();

    if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
    }


}
