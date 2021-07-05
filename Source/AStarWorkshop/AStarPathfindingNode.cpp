// Fill out your copyright notice in the Description page of Project Settings.

#include "AStarPathfindingNode.h"
#include "DrawDebugHelpers.h"

// Sets default values
AAStarPathfindingNode::AAStarPathfindingNode()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);
}

void AAStarPathfindingNode::BeginPlay() 
{
	Super::BeginPlay();

	DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 32, FColor::Green, true);

	for (AAStarPathfindingNode* Node : Neighbours)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), Node->GetActorLocation(), FColor::Green, true);
	}
}