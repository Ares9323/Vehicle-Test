// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "VehicleTestHud.generated.h"


UCLASS(config = Game)
class AVehicleTestHud : public AHUD
{
	GENERATED_BODY()

public:
	AVehicleTestHud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
