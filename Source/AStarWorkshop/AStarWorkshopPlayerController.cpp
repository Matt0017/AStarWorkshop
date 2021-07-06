// Copyright Epic Games, Inc. All Rights Reserved.

#include "AStarWorkshopPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "AStarWorkshopCharacter.h"
#include "Engine/World.h"
#include "NavigationPath.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "AStarPathfindingNode.h"

AAStarWorkshopPlayerController::AAStarWorkshopPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AAStarWorkshopPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
	if (bMoveToMouseCursor)
	{
		MoveToMouseCursor();
	}

	if (DesiredPath.Num() > 0 && UAIBlueprintHelperLibrary::GetCurrentPath(this) == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Entered lol"));
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, DesiredPath[0]);
		DesiredPath.RemoveAt(0);
	}
}

void AAStarWorkshopPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &AAStarWorkshopPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &AAStarWorkshopPlayerController::OnSetDestinationReleased);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AAStarWorkshopPlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AAStarWorkshopPlayerController::MoveToTouchLocation);

	InputComponent->BindAction("ResetVR", IE_Pressed, this, &AAStarWorkshopPlayerController::OnResetVR);
}

void AAStarWorkshopPlayerController::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AAStarWorkshopPlayerController::MoveToMouseCursor()
{
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		if (AAStarWorkshopCharacter* MyPawn = Cast<AAStarWorkshopCharacter>(GetPawn()))
		{
			if (MyPawn->GetCursorToWorld())
			{
				//UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, MyPawn->GetCursorToWorld()->GetComponentLocation());
				AStarMoveTowardsLocation(MyPawn->GetCursorToWorld()->GetComponentLocation());
			}
		}
	}
	else
	{
		// Trace to see what is under the mouse cursor
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		if (Hit.bBlockingHit)
		{
			// We hit something, move there
			SetNewMoveDestination(Hit.ImpactPoint);
		}
	}
}

void AAStarWorkshopPlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
	}
}

void AAStarWorkshopPlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		float const Distance = FVector::Dist(DestLocation, MyPawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if ((Distance > 120.0f))
		{
			//UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, DestLocation);
			AStarMoveTowardsLocation(DestLocation);
		}
	}
}

void AAStarWorkshopPlayerController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void AAStarWorkshopPlayerController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}

void AAStarWorkshopPlayerController::Move(const TArray<FVector>& Positions)
{
	DesiredPath = Positions;
}

void AAStarWorkshopPlayerController::AStarMoveTowardsLocation(const FVector& DestLocation)
{
	RefreshNodes();
	AAStarPathfindingNode* Start = GetNodeFromLocation(GetPawn()->GetActorLocation());
	AAStarPathfindingNode* Goal = GetNodeFromLocation(DestLocation);

	TArray<AAStarPathfindingNode*> OpenList;
	TArray<AAStarPathfindingNode*> CloseList;

	OpenList.Add(Start);

	while (OpenList.Num() > 0)
	{
		AAStarPathfindingNode* Current = GetLowestCostNode(OpenList);
		OpenList.Remove(Current);
		
		if (Current == Goal)
		{
			break;
		}

		for (auto* Neighbour : Current->Neighbours)
		{
			if (CloseList.Contains(Neighbour))
			{
				continue;
			}
			else if (OpenList.Contains(Neighbour))
			{
				float KnownCostThisWay = GetKnownCost(Current, Neighbour);
				if (KnownCostThisWay < Neighbour->KnownCost)
				{
					Neighbour->KnownCost = KnownCostThisWay;
					Neighbour->PreviousNode = Current;
				}
			}
			else
			{
				Neighbour->KnownCost = GetKnownCost(Current, Neighbour);
				Neighbour->HeuristicCost = GetHeuristicCost(Neighbour, Goal);
				Neighbour->PreviousNode = Current;
				OpenList.Add(Neighbour);
			}
		}

		CloseList.Add(Current);
	}

	Move(GenerateSolution(Start, Goal));
}

AAStarPathfindingNode* AAStarWorkshopPlayerController::GetLowestCostNode(const TArray<AAStarPathfindingNode*>& List) const
{
	AAStarPathfindingNode* Out = List[0];
	float Cost = List[0]->TotalCost();
	for (auto* Node : List)
	{
		if (Node->TotalCost() < Cost)
		{
			Cost = Node->TotalCost();
			Out = Node;
		}
	}
	return Out;
}

TArray<FVector> AAStarWorkshopPlayerController::GenerateSolution(AAStarPathfindingNode* Start, AAStarPathfindingNode* Goal)
{
	TArray<FVector> Positions;
	AAStarPathfindingNode* Current = Goal;

	while (Current != Start)
	{
		Positions.Add(Current->GetActorLocation());
		Current = Current->PreviousNode;
	}

	Algo::Reverse(Positions);
	return Positions;
}

float AAStarWorkshopPlayerController::GetKnownCost(AAStarPathfindingNode* Previous, AAStarPathfindingNode* Current)
{
	return Current->KnownCost + FVector::DistSquared(Previous->GetActorLocation(), Current->GetActorLocation()) * Current->Weight;
}

float AAStarWorkshopPlayerController::GetHeuristicCost(AAStarPathfindingNode* Current, AAStarPathfindingNode* Goal)
{
	return FVector::DistSquared(Current->GetActorLocation(), Goal->GetActorLocation());
}

AAStarPathfindingNode* AAStarWorkshopPlayerController::GetNodeFromLocation(const FVector& DestLocation) const
{
	AActor* OutNode = nullptr;
	TArray<AActor*> Nodes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAStarPathfindingNode::StaticClass(), Nodes);

	float MinDistance = MAX_FLT;

	//We'll directly use squared euclidean distance
	for (AActor* Node : Nodes)
	{
		float Distance = FVector::DistSquared(Node->GetActorLocation(), DestLocation);
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			OutNode = Node;
		}
	}

	return Cast<AAStarPathfindingNode>(OutNode);
}

void AAStarWorkshopPlayerController::RefreshNodes()
{
	TArray<AActor*> Nodes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAStarPathfindingNode::StaticClass(), Nodes);
	
	for (AActor* Node : Nodes)
	{
		if (AAStarPathfindingNode* AStatNode = Cast<AAStarPathfindingNode>(Node))
		{
			AStatNode->KnownCost = 0;
			AStatNode->HeuristicCost = 0;
			AStatNode->PreviousNode = nullptr;
		}
	}
}