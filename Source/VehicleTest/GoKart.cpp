// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("MovementComponent"));

}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		NetUpdateFrequency = RefreshFreq;
	}
}

void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoKart,ServerState);
}

FString GetEnumText(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";
	case ROLE_SimulatedProxy:
		return "SimulatedProxy";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default:
		return "Error";
	}
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(MovementComponent == nullptr) return;

	if(GetLocalRole() == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		MovementComponent->SimulateMove(Move);
		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
	}

	//If we are the Server and in control of the Pawn
	if(GetLocalRole() == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	//If we are the Simulated Proxy
	if(GetLocalRole() == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
	}

	DrawDebugString(GetWorld(),FVector(0, 0, 100),GetEnumText(GetLocalRole()),this,FColor::White,DeltaTime);
	UE_LOG(LogTemp, Warning, TEXT("Queue length: %d"), UnacknowledgedMoves.Num());

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
	if(MovementComponent == nullptr) return;
	MovementComponent->SetThrottle(Value);
}

void AGoKart::MoveRight(float Value)
{
	if(MovementComponent == nullptr) return;
	MovementComponent->SetSteeringThrow(Value);
}

void AGoKart::ClearAcknowledgeMoves(FGoKartMove LastMove)
{
	TArray<FGoKartMove> NewMoves;
	for(const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if(Move.TimeStamp>LastMove.TimeStamp)
		{
			NewMoves.Add(Move);
		}
	}
	UnacknowledgedMoves = NewMoves;
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	if(MovementComponent == nullptr) return;

	MovementComponent->SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	if(MovementComponent == nullptr) return false;
	return FMath::Abs(MovementComponent->GetThrottle()) <=1 && FMath::Abs(MovementComponent->GetSteeringThrow()) <=1;
}

void AGoKart::OnRep_RepServerState()
{
	if(MovementComponent == nullptr) return;
	//UE_LOG(LogTemp, Warning, TEXT("Replicated Transform"));
	SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);
	ClearAcknowledgeMoves(ServerState.LastMove);
	for(const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}