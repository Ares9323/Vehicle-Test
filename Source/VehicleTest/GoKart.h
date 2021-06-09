// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKartMovementComponent.h"
#include "GoKart.generated.h"

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

	void ClearAcknowledgeMoves(FGoKartMove LastMove);

	UPROPERTY(ReplicatedUsing=OnRep_RepServerState)
	FGoKartState ServerState;

	UFUNCTION()
	void OnRep_RepServerState();

	TArray<FGoKartMove> UnacknowledgedMoves;

	UPROPERTY(EditAnywhere)
	UGoKartMovementComponent* MovementComponent;

};
