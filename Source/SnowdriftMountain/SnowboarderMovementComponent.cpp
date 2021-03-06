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

    
    // FString wasMode = StaticEnum<EMovementMode>()->GetNameStringByValue(static_cast<int32>(PreviousMovementMode));
    // FString iisMode = StaticEnum<EMovementMode>()->GetNameStringByValue(static_cast<int32>(MovementMode));
    // GEngine->AddOnScreenDebugMessage(-1, 3., FColor::Cyan, "Move mode changed from " + wasMode + " to " + iisMode);
    
    if (MovementMode == EMovementMode::MOVE_Custom) {
        GEngine->AddOnScreenDebugMessage(-1, 3., FColor::Cyan, "velocity when landing: " + Velocity.ToCompactString());
    }

}

bool USnowboarderMovementComponent::CanAttemptJump() const
{
    // may want to change this is we want to allow "crouching" on the board
    return IsJumpAllowed() && !bWantsToCrouch && (IsMovingOnGround() || IsFalling()); ;
}
bool USnowboarderMovementComponent::IsMovingOnGround() const
{
    return ((MovementMode == MOVE_Walking) || (MovementMode == MOVE_Custom)) && UpdatedComponent;
}

void USnowboarderMovementComponent::PhysBoard(float deltaTime, int32 Iterations)
{
    
    if (deltaTime < MIN_TICK_TIME)
    {
        return;
    }

    Iterations++;
    bJustTeleported = false;

    const FVector OldLocation = UpdatedComponent->GetComponentLocation();
    const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();

    //MaintainHorizontalGroundVelocity();

    // Attempt to move:
    FStepDownResult stepDown;
    MoveAlongFloor(Velocity, deltaTime, &stepDown);

    // Update floor
    if (stepDown.bComputedFloor)
    {
        CurrentFloor = stepDown.FloorResult;
    } else {
        FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, false, NULL);
    }

    if (CurrentFloor.IsWalkableFloor())
    {
        //////////////////////////////////////// DO SLIDING:

        if (!CurrentFloor.HitResult.bStartPenetrating) {
            if (CurrentFloor.bBlockingHit) {

                const FVector OldHitNormal = CurrentFloor.HitResult.Normal;
                const FVector OldHitImpactNormal = CurrentFloor.HitResult.ImpactNormal;

                const FVector ProjectedNormal = ConstrainNormalToPlane(CurrentFloor.HitResult.Normal);
                FVector Delta = FVector::VectorPlaneProject(FVector(0, 0, -1), ProjectedNormal) * deltaTime;

                TwoWallAdjust(Delta, CurrentFloor.HitResult, OldHitNormal);

            }
            else {
                GEngine->AddOnScreenDebugMessage(-1, 3., FColor::Cyan, "floor NOT blocking?");
            }
        }
        else {
            GEngine->AddOnScreenDebugMessage(-1, 3., FColor::Cyan, "pentrating floor?");
        }

    } else {
        GEngine->AddOnScreenDebugMessage(-1, 3., FColor::Cyan, "bad floor, start falling");
        SetMovementMode(MOVE_Falling);
    }

 
    // this clamps the velocity
    // however, it also seems to cause a hiccup when the player lands... 
    //Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation).GetSafeNormal() * Velocity.Size();

}

void USnowboarderMovementComponent::AccelerateDownHill(FVector boardForward, float leanUphill, float dt) {


    if (MovementMode != MOVE_Custom) return;

    // which way is down hill?
    FVector dirDownhill = FVector::VectorPlaneProject(FVector(0, 0, -1), CurrentFloor.HitResult.Normal);
 
    // how much the board is aligned with the downhill direction
    float alignDownHill = FVector::DotProduct(boardForward.GetSafeNormal2D(), dirDownhill.GetSafeNormal2D());
    if (alignDownHill < 0) alignDownHill *= -1;

    // how much the board is aligned with the Velocity
    FVector dirVelocity = Velocity.GetSafeNormal();
    float alignVelocity = FVector::DotProduct(boardForward.GetSafeNormal(), dirVelocity);
    FVector dirBoard = (alignVelocity < 0) ? -boardForward : boardForward;
    if (alignVelocity < 0) alignVelocity *= -1;

    alignVelocity *= alignVelocity;
    alignVelocity *= alignVelocity;

    // how much the velocity is aligned with Down
    float alignDown = FVector::DotProduct(dirVelocity, FVector(0, 0, -1));
    //if (alignDown < .01f) alignDown = .01f;

    alignDown *= (alignDown < 0) ? -alignDown : alignDown; // keep negative numbers negative

    float slope = FMath::Acos(CurrentFloor.HitResult.Normal.Z);

    //GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Blue, "slope: " + FString::SanitizeFloat(slope));

    FVector vel = Velocity;
    vel += dt * dirDownhill * alignDownHill * slope * slope * 10000;//FMath::Lerp(10000, 100, leanUphill);
    vel += dt * dirBoard * alignVelocity * alignDown * 10000;//FMath::Lerp(0, 10000, leanUphill);

    // clamp:
    const float maxSpeed = 2000.f;
    if (vel.SizeSquared() > maxSpeed * maxSpeed) vel = vel * maxSpeed / vel.Size();
    Velocity = vel;
}
