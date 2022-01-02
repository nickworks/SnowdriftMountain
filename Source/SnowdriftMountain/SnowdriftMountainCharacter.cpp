// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowdriftMountainCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// ASnowdriftMountainCharacter
//ASnowdriftMountainCharacter::ASnowdriftMountainCharacter() {
ASnowdriftMountainCharacter::ASnowdriftMountainCharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USnowboarderMovementComponent>(ACharacter::CharacterMovementComponentName)){

	StatePrimaryPhys = EBoarderState::InAir;


	// Set up TheRoot (small sphere):
	//TheRoot = CreateDefaultSubobject<USphereComponent>(TEXT("TheRoot"));
	//TheRoot->InitSphereRadius(5.f);
	//TheRoot->SetupAttachment(RootComponent);
	
	//MoveComponent = CreateDefaultSubobject<USnowboarderMovementComponent>(FName("MoveComponent"));
	//MoveComponent->UpdatedComponent = TheRoot;
	
	/*
	// Set size for collision capsule
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(42.f, 96.0f);
	Capsule->SetRelativeLocation(FVector(0, 0, 96.f));
	Capsule->SetupAttachment(TheRoot);
	*/

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	//GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	//GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	//GetCharacterMovement()->JumpZVelocity = 600.f;
	//GetCharacterMovement()->AirControl = 0.2f;

	BoardRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BoardRoot"));
	BoardRoot->SetupAttachment(RootComponent);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASnowdriftMountainCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASnowdriftMountainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASnowdriftMountainCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASnowdriftMountainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASnowdriftMountainCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ASnowdriftMountainCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ASnowdriftMountainCharacter::TouchStopped);

	PlayerInputComponent->BindAction("ToggleBoard", IE_Released, this, &ASnowdriftMountainCharacter::ToggleBoard);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASnowdriftMountainCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASnowdriftMountainCharacter::StopJump);
}

void ASnowdriftMountainCharacter::Tick(float dt)
{

	Super::Tick(dt);

	auto *move = GetSnowboardMovement();
	if (move){// && move->MovementMode == MOVE_Custom) {
		if(move->CurrentFloor.IsWalkableFloor()) BoardRoot->SetWorldLocation(move->CurrentFloor.HitResult.ImpactPoint);
		Raycast2();
		BoardRoot->SetRelativeRotation(UKismetMathLibrary::RInterpTo(BoardRoot->GetRelativeRotation(), rotBoard, dt, 10.f));
		move->AccelerateDownHill(BoardRoot->GetForwardVector(), 1, dt);
	}

}

USnowboarderMovementComponent* ASnowdriftMountainCharacter::GetSnowboardMovement()
{

	auto *mover = Cast<USnowboarderMovementComponent>(GetCharacterMovement());

	return (mover) ? mover : nullptr;
}

void ASnowdriftMountainCharacter::Raycast2()
{
	float boardHalfWidth = 30.f;
	float boardHalfLength = 80.f;

	float capsuleHalfHeight = 0.f;//GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float raiseCastOrigins = 250.f;


	FVector offset = FVector(0, 0, raiseCastOrigins);
	FVector normal = FVector(0, 0, 0);
	float avgDis = 0;
	int count = 0;

	StatePrimaryPhys = EBoarderState::InAir;
	FTransform xform = BoardRoot->GetComponentTransform();

	FVector disToEnd = FVector(0, 0, -1000);
	ECollisionChannel channel = ECollisionChannel::ECC_GameTraceChannel1;

	FHitResult hitBL;
	FHitResult hitBR;
	FHitResult hitFL;
	FHitResult hitFR;

	FVector startBL = xform.TransformPosition(FVector(-boardHalfLength, -boardHalfWidth, 0));
	FVector startBR = xform.TransformPosition(FVector(-boardHalfLength, +boardHalfWidth, 0));
	FVector startFL = xform.TransformPosition(FVector(+boardHalfLength, -boardHalfWidth, 0));
	FVector startFR = xform.TransformPosition(FVector(+boardHalfLength, +boardHalfWidth, 0));

	startBL.Z = startBR.Z = startFL.Z = startFR.Z = BoardRoot->GetComponentLocation().Z + raiseCastOrigins;


	bool didHitBL = GetWorld()->LineTraceSingleByChannel(hitBL, startBL, startBL + disToEnd, channel);
	bool didHitBR = GetWorld()->LineTraceSingleByChannel(hitBR, startBR, startBR + disToEnd, channel);
	bool didHitFL = GetWorld()->LineTraceSingleByChannel(hitFL, startFL, startFL + disToEnd, channel);
	bool didHitFR = GetWorld()->LineTraceSingleByChannel(hitFR, startFR, startFR + disToEnd, channel);

	UKismetSystemLibrary::DrawDebugLine(GetWorld(), startBL, startBL + disToEnd, FColor::Yellow, 0, 1);
	UKismetSystemLibrary::DrawDebugLine(GetWorld(), startBR, startBR + disToEnd, FColor::Yellow, 0, 1);
	UKismetSystemLibrary::DrawDebugLine(GetWorld(), startFL, startFL + disToEnd, FColor::Yellow, 0, 1);
	UKismetSystemLibrary::DrawDebugLine(GetWorld(), startFR, startFR + disToEnd, FColor::Yellow, 0, 1);

	float avgDisBack = 0;
	if (didHitBL && didHitBR) avgDisBack = (hitBL.Distance + hitBR.Distance) / 2;
	else if (didHitBL) avgDisBack = hitBL.Distance;
	else if (didHitBR) avgDisBack = hitBR.Distance;
	
	float avgDisFront = 0;
	if (didHitFL && didHitFR) avgDisFront = (hitFL.Distance + hitFR.Distance) / 2;
	else if (didHitFL) avgDisFront = hitFL.Distance;
	else if (didHitFR) avgDisFront = hitFR.Distance;
	
	float avgDisLeft = 0;
	if (didHitBL && didHitFL) avgDisLeft = (hitFL.Distance + hitBL.Distance) / 2;
	else if (didHitBL) avgDisLeft = hitBL.Distance;
	else if (didHitFL) avgDisLeft = hitFL.Distance;

	float avgDisRight = 0;
	if (didHitFR && didHitBR) avgDisRight = (hitFR.Distance + hitBR.Distance) / 2;
	else if (didHitFR) avgDisRight = hitFR.Distance;
	else if (didHitBR) avgDisRight = hitBR.Distance;

	float pitch = 0;
	float roll = 0;

	if(avgDisBack > 0 && avgDisFront > 0) pitch = FMath::RadiansToDegrees(FMath::Atan((avgDisBack - avgDisFront) / (boardHalfLength * 2)));
	if(avgDisLeft > 0 && avgDisRight > 0) roll = FMath::RadiansToDegrees(FMath::Atan((avgDisRight - avgDisLeft) / (boardHalfWidth * 2)));

	FMath::WindRelativeAnglesDegrees(rotBoard.Roll, roll);
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::SanitizeFloat(roll));
	rotBoard = (FQuat(FRotator(pitch, 0, 0)) * FQuat(FRotator(0, 0, roll))).Rotator();

	float thresh = capsuleHalfHeight + raiseCastOrigins + 10.f;
	if (avgDisBack == 0 || avgDisFront == 0) return;
	if (avgDisBack < thresh || avgDisFront < thresh) StatePrimaryPhys = EBoarderState::OnGround;
}

void ASnowdriftMountainCharacter::BeginPlay()
{
	Super::BeginPlay();
}
void ASnowdriftMountainCharacter::ToggleBoard() {
	auto *move = GetSnowboardMovement();
	if (!move) return;

	if (!move->IsMovingOnGround()) return;
	
	// are we walking?
	bool isWalk = (move->DefaultLandMovementMode == MOVE_Walking);
	// flip it:
	isWalk = !isWalk;

	// update some values:
	move->DefaultLandMovementMode = isWalk ? MOVE_Walking : MOVE_Custom;
	BoardRoot->SetVisibility(isWalk ? false : true, true); 
	move->SetWalkableFloorAngle(isWalk ? 80 : 90);

	// change mode:
	move->SetMovementMode(move->DefaultLandMovementMode);
}

void ASnowdriftMountainCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		//Jump();
}

void ASnowdriftMountainCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		//StopJumping();
}

void ASnowdriftMountainCharacter::StartJump()
{

	Jump();
}

void ASnowdriftMountainCharacter::StopJump()
{
	StopJumping();
}

void ASnowdriftMountainCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASnowdriftMountainCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASnowdriftMountainCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		/**/
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
		/**/
	}
}

void ASnowdriftMountainCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		/*
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
		*/
		//TheRoot->AddTorqueInRadians(FVector(0, 0, 1) * Value * 500 * GetWorld()->GetDeltaSeconds(), NAME_None, false);
		AddActorLocalRotation(FRotator(0, Value * 90 * GetWorld()->GetDeltaSeconds(), 0));

	}
}
