// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceShooter2022Pawn.h"
#include "SpaceShooter2022Projectile.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

const FName ASpaceShooter2022Pawn::MoveForwardBinding("MoveForward");
const FName ASpaceShooter2022Pawn::MoveRightBinding("MoveRight");
const FName ASpaceShooter2022Pawn::FireForwardBinding("FireForward");
const FName ASpaceShooter2022Pawn::FireRightBinding("FireRight");
//const FName ASpaceShooter2022Pawn::FireBinding("Fire");

ASpaceShooter2022Pawn::ASpaceShooter2022Pawn()
{	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/TwinStick/Meshes/TwinStickUFO.TwinStickUFO"));
	// Create the mesh component
	ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMeshComponent;
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(ShipMesh.Object);
	
	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/TwinStick/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Create a camera boom...
	//CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//CameraBoom->SetupAttachment(RootComponent);
	//CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when ship does
	//CameraBoom->TargetArmLength = 1200.f;
	//CameraBoom->SetRelativeRotation(FRotator(-80.f, 0.f, 0.f));
	//CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	//CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	//CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//CameraComponent->bUsePawnControlRotation = false;	// Camera does not rotate relative to arm

	// Movement
	MoveSpeed = 1000.0f;
	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 0.1f;
	bCanFire = true;
	rotateAmount = 0.0f;
	shooting = false;
}

void ASpaceShooter2022Pawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// set up gameplay key bindings
	PlayerInputComponent->BindAxis(MoveForwardBinding);
	PlayerInputComponent->BindAxis(MoveRightBinding);
	PlayerInputComponent->BindAxis(FireForwardBinding);
	PlayerInputComponent->BindAxis(FireRightBinding);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ASpaceShooter2022Pawn::Fire);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &ASpaceShooter2022Pawn::UnFire);
}

void ASpaceShooter2022Pawn::Tick(float DeltaSeconds)
{
	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	const float RightValue = GetInputAxisValue(MoveRightBinding);

	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	const FVector MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

	// Calculate  movement
	const FVector Movement = MoveDirection * MoveSpeed * DeltaSeconds;

	if (RightValue != 0.0f) {
		rotateAmount += RightValue * RotateSpeed * DeltaSeconds;
		rotateAmount = FMath::Clamp(rotateAmount, -RotationMax, RotationMax);
	}
	else if (rotateAmount < 0.0f) {
		rotateAmount += RotateSpeed * DeltaSeconds;
		rotateAmount = FMath::Min(rotateAmount, 0.0f);
	}
	else if (rotateAmount > 0.0f) {
		rotateAmount -= RotateSpeed * DeltaSeconds;
		rotateAmount = FMath::Max(rotateAmount, 0.0f);
	}

	// If non-zero size, move this actor
	//if (Movement.SizeSquared() > 0.0f)
	//{
		const FRotator NewRotation = FRotator(0.f, 0.f, rotateAmount);
		FHitResult Hit(1.f);
		RootComponent->MoveComponent(Movement, NewRotation, true, &Hit);
		
		if (Hit.IsValidBlockingHit())
		{
			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
			RootComponent->MoveComponent(Deflection, NewRotation, true);
		}
	//}
	
	// Create fire direction vector
	//const float FireForwardValue = GetInputAxisValue(FireForwardBinding);
	//const float FireRightValue = GetInputAxisValue(FireRightBinding);
	//const bool FireValue = GetInputAction
	//
	//// Try and fire a shot
	//if (FireForwardValue != 0.f || FireForwardValue != 0.f || FireValue) {
	//	const FVector FireDirection = FVector(1.f, 0.f, 0.f);
	//	FireShot(FireDirection);
	//}
	if (shooting) FireShot(ShootDirection);
}

void ASpaceShooter2022Pawn::FireShot(FVector FireDirection)
{
	// If it's ok to fire again
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{
			const FRotator FireRotation = FireDirection.Rotation();
			// Spawn projectile at an offset from this pawn
			const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

			UWorld* const World = GetWorld();
			if (World != nullptr)
			{
				// spawn the projectile
				World->SpawnActor<ASpaceShooter2022Projectile>(SpawnLocation, FireRotation);
			}

			bCanFire = false;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &ASpaceShooter2022Pawn::ShotTimerExpired, FireRate);

			// try and play the sound if specified
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}

			bCanFire = false;
		}
	}
}

void ASpaceShooter2022Pawn::Fire() 
{
	shooting = true;
}

void ASpaceShooter2022Pawn::UnFire()
{
	shooting = false;
}

void ASpaceShooter2022Pawn::ShotTimerExpired()
{
	bCanFire = true;
}

