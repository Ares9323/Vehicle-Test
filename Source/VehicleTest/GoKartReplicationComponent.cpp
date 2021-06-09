// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartReplicationComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UGoKartReplicationComponent::UGoKartReplicationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}


// Called when the game starts
void UGoKartReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();

}


// Called every frame
void UGoKartReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(MovementComponent == nullptr) return;

	FGoKartMove LastMove = MovementComponent->GetLastMove();

	if(GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(LastMove);
		Server_SendMove(LastMove);
	}

	//If we are the Server and in control of the Pawn
	if(GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(LastMove);
	}

	//If we are the Simulated Proxy
	if(GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}

	UE_LOG(LogTemp, Warning, TEXT("Queue length: %d"), UnacknowledgedMoves.Num());
}


void UGoKartReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGoKartReplicationComponent,ServerState);
}

void UGoKartReplicationComponent::ClearAcknowledgeMoves(FGoKartMove LastMove)
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

void UGoKartReplicationComponent::UpdateServerState(const FGoKartMove &Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

void UGoKartReplicationComponent::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;
	if(ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return;
	if(MovementComponent == nullptr) return;

	float LerpRatio = ClientTimeSinceUpdate/ClientTimeBetweenLastUpdates;
	FHermiteCubicSpline Spline = CreateSpline();

// Derivative interpolation

	InterpolateRotation(LerpRatio);
	InterpolateLocation(Spline,LerpRatio);
	InterpolateVelocity(Spline,LerpRatio);

}

void UGoKartReplicationComponent::InterpolateLocation(const FHermiteCubicSpline &Spline, float LerpRatio)
{
	FVector NewLocation = Spline.InterpolateLocation(LerpRatio);
	GetOwner()->SetActorLocation(NewLocation);
}

void UGoKartReplicationComponent::InterpolateVelocity(const FHermiteCubicSpline &Spline, float LerpRatio)
{
	FVector NewDerivative = Spline.InterpolateDerivative(LerpRatio);
	FVector NewVelocity = NewDerivative / VelocityToDerivative();
	MovementComponent->SetVelocity(NewVelocity);
}

void UGoKartReplicationComponent::InterpolateRotation(float LerpRatio)
{
	FQuat StartRotation = ClientStartTransform.GetRotation();
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat NewRotation = FQuat::Slerp(StartRotation,TargetRotation,LerpRatio);
	GetOwner()->SetActorRotation(NewRotation);
}

FHermiteCubicSpline UGoKartReplicationComponent::CreateSpline()
{
	FHermiteCubicSpline Spline;
	Spline.StartLocation = ClientStartTransform.GetLocation();
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartDerivative = ClientStartVelocity * VelocityToDerivative();
	Spline.TargetDerivative = ServerState.Velocity * VelocityToDerivative();

	return Spline;
}

float UGoKartReplicationComponent::VelocityToDerivative()
{
	return ClientTimeBetweenLastUpdates * 100;
}

void UGoKartReplicationComponent::Server_SendMove_Implementation(FGoKartMove Move)
{
	if(MovementComponent == nullptr) return;

	MovementComponent->SimulateMove(Move);
	UpdateServerState(Move);
}

bool UGoKartReplicationComponent::Server_SendMove_Validate(FGoKartMove Move)
{
	if(MovementComponent == nullptr) return false;
	return FMath::Abs(MovementComponent->GetThrottle()) <=1 && FMath::Abs(MovementComponent->GetSteeringThrow()) <=1;
}

void UGoKartReplicationComponent::OnRep_RepServerState()
{
	if(MovementComponent == nullptr) return;
	//UE_LOG(LogTemp, Warning, TEXT("Replicated Transform"));
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_RepServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_RepServerState();
		break;
	default:
		break;
	}
}

void UGoKartReplicationComponent::AutonomousProxy_OnRep_RepServerState()
{
	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);
	ClearAcknowledgeMoves(ServerState.LastMove);
	for(const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UGoKartReplicationComponent::SimulatedProxy_OnRep_RepServerState()
{
	if(MovementComponent == nullptr) return;
	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;

	ClientStartTransform = GetOwner()->GetActorTransform();
	ClientStartVelocity = MovementComponent->GetVelocity();
}
