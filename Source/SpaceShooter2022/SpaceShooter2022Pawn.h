// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpaceShooter2022Pawn.generated.h"

UCLASS(Blueprintable)
class ASpaceShooter2022Pawn : public APawn
{
	GENERATED_BODY()

	/* The mesh component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ShipMeshComponent;

	/** The camera */
	//UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//class UCameraComponent* CameraComponent;

	/** Camera boom positioning the camera above the character */
	//UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//class USpringArmComponent* CameraBoom;

public:
	ASpaceShooter2022Pawn();

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float MaxHealth;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float CurrentHealth;

	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite )
	FVector GunOffset;

	//Shoot Direction
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	FVector ShootDirection;
	
	/* How fast the weapon will fire */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float FireRate;

	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float MoveSpeed;

	//rotate speed of the ship
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float RotateSpeed;

	//max rotation
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float RotationMax;

	//clamp player movement
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	FVector MaxPosition;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	FVector MinPosition;

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	class USoundBase* FireSound;

	// Begin Actor Interface
	//virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End Actor Interface

	/* Fire a shot in the specified direction */
	//UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintCallable)
	UFUNCTION(BlueprintCallable)
	void FireShot(FVector FireDirection);

	UFUNCTION(BlueprintCallable)
	void MovePlayer(float DeltaSeconds, FVector MoveDirection);

	/* Handler for the fire timer expiry */
	void ShotTimerExpired();

	// Static names for axis bindings
	static const FName MoveForwardBinding;
	static const FName MoveRightBinding;
	static const FName FireForwardBinding;
	static const FName FireRightBinding;

	//Static names for Action Bindings
	//static const FName FireBinding;
	 
	//fire input event
	UFUNCTION(BlueprintCallable)
	void Fire();

	UFUNCTION(BlueprintCallable)
	void UnFire();

private:

	/* Flag to control firing  */
	uint32 bCanFire : 1;
	float rotateAmount;
	bool shooting;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

public:
	/** Returns ShipMeshComponent subobject **/
	FORCEINLINE class UStaticMeshComponent* GetShipMeshComponent() const { return ShipMeshComponent; }
	/** Returns CameraComponent subobject **/
	//FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	/** Returns CameraBoom subobject **/
	//FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};

