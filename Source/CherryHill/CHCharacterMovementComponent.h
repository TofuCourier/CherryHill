// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CHCharacterMovementComponent.generated.h"

class ACherryHillCharacter;

UCLASS()
class CHERRYHILL_API UCHCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

protected:

	UCHCharacterMovementComponent();

	virtual void BeginPlay() override;

	virtual void PhysFalling(float DeltaTime, int32 Iterations) override;

	virtual void PhysFlying(float deltaTime, int32 Iterations) override;


	ACherryHillCharacter* MyOwner = nullptr;
	
};
