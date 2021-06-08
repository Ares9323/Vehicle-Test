// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;

	//Drag force of the fluid
	Force += GetFluidResistance();
	Force += GetRollingResistance();

	//Acceleration = Force / Mass
	FVector Acceleration = Force / Mass;

	//Velocity = InitialVelocity + Acceleration * Time
	Velocity = Velocity + Acceleration * DeltaTime;


	ApplyRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);

	UE_LOG(LogTemp, Warning, TEXT("Velocity: %s"),*Velocity.ToString());
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::MoveForward(float Value)
{
	Throttle = Value;
}

void AGoKart::MoveRight(float Value)
{
	SteeringThrow = Value;
}

FVector AGoKart::GetFluidResistance()
{
	return - Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector AGoKart::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = Mass * AccelerationDueToGravity;
	return - Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	//Space = Velocity * Time
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult Hit;
	AddActorWorldOffset(Translation, true, &Hit);

	//Set velocity to zero if hit is blocking movement
	if(Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

void AGoKart::ApplyRotation(float DeltaTime)
{
	float DeltaLocation = Velocity.Size() * DeltaTime;
	float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;

	FQuat RotationDelta(GetActorUpVector(), RotationAngle);
	//Rotate the velocity vector
	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorWorldRotation(RotationDelta);
}