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

//const FName ASpaceShooter2022Pawn::MoveForwardBinding("MoveForward");
//const FName ASpaceShooter2022Pawn::MoveRightBinding("MoveRight");
//const FName ASpaceShooter2022Pawn::FireForwardBinding("FireForward");
//const FName ASpaceShooter2022Pawn::FireRightBinding("FireRight");
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

	//health
	CurrentHealth = MaxHealth;
}

//void ASpaceShooter2022Pawn::Tick(float DeltaSeconds) {}

void ASpaceShooter2022Pawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// set up gameplay key bindings
	//PlayerInputComponent->BindAxis(MoveForwardBinding);
	//PlayerInputComponent->BindAxis(MoveRightBinding);
	//PlayerInputComponent->BindAxis(FireForwardBinding);
	//PlayerInputComponent->BindAxis(FireRightBinding);
	//PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ASpaceShooter2022Pawn::Fire);
	//PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &ASpaceShooter2022Pawn::UnFire);
}

void ASpaceShooter2022Pawn::MovePlayer(float DeltaSeconds, FVector MoveDirection)
{
	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	MoveDirection = MoveDirection.GetClampedToMaxSize(1.0f);

	

	// Calculate  movement
	const FVector Movement = MoveDirection * MoveSpeed * DeltaSeconds;

	//rotate the ship
	if (MoveDirection.X != 0.0f) {
		rotateAmount += MoveDirection.X * RotateSpeed * DeltaSeconds;
		rotateAmount = FMath::Clamp(rotateAmount, -RotationMax, RotationMax);
	}

	//rotate it back if not moving
	else if (rotateAmount < 0.0f) {
		rotateAmount += RotateSpeed * DeltaSeconds;
		rotateAmount = FMath::Min(rotateAmount, 0.0f);
	}
	else if (rotateAmount > 0.0f) {
		rotateAmount -= RotateSpeed * DeltaSeconds;
		rotateAmount = FMath::Max(rotateAmount, 0.0f);
	}

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, Movement.ToString());

	const FRotator NewRotation = FRotator(0.f, 0.f, rotateAmount);
	FHitResult Hit(1.f);
	RootComponent->MoveComponent(Movement, NewRotation, true, &Hit);

	//if (Hit.IsValidBlockingHit())
	//{
	//	const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
	//	const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
	//	RootComponent->MoveComponent(Deflection, NewRotation, true);
	//}

	//get world position
	FVector currentLocation = RootComponent->GetComponentLocation();

	//clamp position
	currentLocation.X = FMath::Clamp(currentLocation.X, MinPosition.X, MaxPosition.X);
	currentLocation.Y = FMath::Clamp(currentLocation.Y, MinPosition.Y, MaxPosition.Y);
	currentLocation.Z = FMath::Clamp(currentLocation.Z, MinPosition.Z, MaxPosition.Z);

	//set new position
	RootComponent->SetWorldLocation(currentLocation, false, &Hit);
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

