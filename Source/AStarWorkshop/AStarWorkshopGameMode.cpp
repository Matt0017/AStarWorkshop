// Copyright Epic Games, Inc. All Rights Reserved.

#include "AStarWorkshopGameMode.h"
#include "AStarWorkshopPlayerController.h"
#include "AStarWorkshopCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAStarWorkshopGameMode::AAStarWorkshopGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AAStarWorkshopPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}