// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"


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

	bool IsValid() const
	{
		return FMath::Abs(Throttle) <= 1 && FMath::Abs(SteeringThrow) <= 1;
	}

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VEHICLETEST_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGoKartMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Getters/Setters
	FORCEINLINE void SetThrottle(float Value) {Throttle = Value;}
	FORCEINLINE float GetThrottle() {return Throttle;}

	FORCEINLINE void SetSteeringThrow(float Value) {SteeringThrow = Value;}
	FORCEINLINE float GetSteeringThrow() {return SteeringThrow;}

	FORCEINLINE void SetVelocity(FVector Value) {Velocity = Value;}
	FORCEINLINE FVector GetVelocity() {return Velocity;}

	FORCEINLINE FGoKartMove GetLastMove() {return LastMove;}

	void SimulateMove(const FGoKartMove& Move);

private:
	FGoKartMove CreateMove(float DeltaTime);

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

	UPROPERTY()
	FVector Velocity;

	float Throttle;
	float SteeringThrow;

	FGoKartMove LastMove;
};
