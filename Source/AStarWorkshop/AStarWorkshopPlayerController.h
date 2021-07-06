// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AStarWorkshopPlayerController.generated.h"

class AAStarPathfindingNode;

UCLASS()
class AAStarWorkshopPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAStarWorkshopPlayerController();

protected:

	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Navigate player to the current mouse cursor location. */
	void MoveToMouseCursor();

	/** Navigate player to the current touch location. */
	void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);
	
	/** Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

	/** Input handlers for SetDestination action. */
	void OnSetDestinationPressed();
	void OnSetDestinationReleased();

	void Move(const TArray<FVector>& Positions);
	void AStarMoveTowardsLocation(const FVector& DestLocation);

	AAStarPathfindingNode* GetLowestCostNode(const TArray<AAStarPathfindingNode*>& List) const;

	float GetKnownCost(AAStarPathfindingNode* Previous, AAStarPathfindingNode* Current);
	float GetHeuristicCost(AAStarPathfindingNode* Current, AAStarPathfindingNode* Goal);

	TArray<FVector> GenerateSolution(AAStarPathfindingNode* Start, AAStarPathfindingNode* Goal);

	AAStarPathfindingNode* GetNodeFromLocation(const FVector& DestLocation) const;

	void RefreshNodes();
	
	TArray<FVector> DesiredPath;
};


