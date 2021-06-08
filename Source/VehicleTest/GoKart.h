// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

UCLASS()
class VEHICLETEST_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:
	void MoveForward(float Value);

	FVector Velocity;

	// The mass of the car in kg
	UPROPERTY(EditAnywhere, Category="Physics")
	float Mass = 1000;

	// The force applied to the car when throttle is fully down (N kg*m/s^2)
	UPROPERTY(EditAnywhere, Category="Physics")
	float MaxDrivingForce = 10000;

	float Throttle;

};
