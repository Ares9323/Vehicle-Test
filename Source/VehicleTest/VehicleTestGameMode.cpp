// Copyright Epic Games, Inc. All Rights Reserved.

#include "VehicleTestGameMode.h"
#include "VehicleTestPawn.h"
#include "VehicleTestHud.h"

AVehicleTestGameMode::AVehicleTestGameMode()
{
	DefaultPawnClass = AVehicleTestPawn::StaticClass();
	HUDClass = AVehicleTestHud::StaticClass();
}
