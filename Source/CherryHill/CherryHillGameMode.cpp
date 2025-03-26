// Copyright Epic Games, Inc. All Rights Reserved.

#include "CherryHillGameMode.h"
#include "CherryHillCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACherryHillGameMode::ACherryHillGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
