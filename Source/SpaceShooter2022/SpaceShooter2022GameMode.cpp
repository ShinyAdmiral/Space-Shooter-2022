// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceShooter2022GameMode.h"
#include "SpaceShooter2022Pawn.h"

ASpaceShooter2022GameMode::ASpaceShooter2022GameMode()
{
	// set default pawn class to our character class
	DefaultPawnClass = ASpaceShooter2022Pawn::StaticClass();
}

