// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CHGameplayInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCHGameplayInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */

 /* Add interface functions to this class. This is the class that will be inherited to implement this interface. */
class CHERRYHILL_API ICHGameplayInterface
{
	GENERATED_BODY()

public:

	/* Called after the Actor State was restored from a SaveGame file*/
	UFUNCTION(BlueprintNativeEvent)
	void OnActorLoaded();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FText GetInteractText(APawn* InstigatorPawn);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Interact(APawn* InstigatorPawn);
};
