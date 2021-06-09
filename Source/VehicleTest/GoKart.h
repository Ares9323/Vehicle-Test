// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float TimeStamp;

};

USTRUCT()
struct FGoKartState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FGoKartMove LastMove;

};


UCLASS()
class VEHICLETEST_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

	// Update frequency
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	int32 RefreshFreq = 1;

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
	void MoveRight(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	void SimulateMove(const FGoKartMove& Move);

	FGoKartMove CreateMove(float DeltaTime);
	void ClearAcknowledgeMoves(FGoKartMove LastMove);

	FVector GetFluidResistance();
	FVector GetRollingResistance();
	void UpdateLocationFromVelocity(float DeltaTime);
	void ApplyRotation(float DeltaTime, float SteeringThrow);

	// The fluid drag coefficient (kg*m)
	UPROPERTY(EditAnywhere, Category="Physics")
	float DragCoefficient = 16;

	// The rolling resistance coefficient of ordinary cars on concrete
	UPROPERTY(EditAnywhere, Category="Physics")
	float RollingResistanceCoefficient = 0.015;

	// The fluid drag coefficient
	UPROPERTY(EditAnywhere, Category="Physics")
	FVector FluidResistance;

	// The mass of the car in kg
	UPROPERTY(EditAnywhere, Category="Physics")
	float Mass = 1000;

	// The force applied to the car when throttle is fully down (N kg*m/s^2)
	UPROPERTY(EditAnywhere, Category="Physics")
	float MaxDrivingForce = 10000;

	// The minimum radius of the car turning circle at full lock (m)
	UPROPERTY(EditAnywhere, Category="Physics")
	float MinTurningRadius = 10;

	UPROPERTY(ReplicatedUsing=OnRep_RepServerState)
	FGoKartState ServerState;

	UFUNCTION()
	void OnRep_RepServerState();

	UPROPERTY()
	FVector Velocity;

	float Throttle;
	float SteeringThrow;

	TArray<FGoKartMove> UnacknowledgedMoves;

};
