// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AStarPathfindingNode.generated.h"

UCLASS()
class ASTARWORKSHOP_API AAStarPathfindingNode : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAStarPathfindingNode();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	int32 Radius = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USceneComponent* SceneComponent;

	UPROPERTY(EditInstanceOnly);
	TArray<AAStarPathfindingNode*> Neighbours;

	UPROPERTY(EditInstanceOnly);
	int32 Weight = 1;
	
	/*Parent */ AAStarPathfindingNode* PreviousNode;

	/*F */ float TotalCost() { return KnownCost + HeuristicCost; }
	/*G */ float KnownCost = 0;
	/*H */ float HeuristicCost = 0;
};