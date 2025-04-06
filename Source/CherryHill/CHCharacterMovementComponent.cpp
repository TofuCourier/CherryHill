// Fill out your copyright notice in the Description page of Project Settings.


#include "CHCharacterMovementComponent.h"
#include "CherryHillCharacter.h"
#include "Math/UnrealMathUtility.h"
#include "CHJetpack.h"
#include "Kismet/KismetMathLibrary.h"

UCHCharacterMovementComponent::UCHCharacterMovementComponent()
{
	MyOwner = Cast<ACherryHillCharacter>(GetOwner());
}

void UCHCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	bOrientRotationToMovement = false;
}

void UCHCharacterMovementComponent::PhysFalling(float DeltaTime, int32 Iterations)
{
 	if (MyOwner->IsJetpackActive() && MyOwner->MyJetpack->IsThrusting()) // Example: your own jetpack flag
	{
		SetMovementMode(MOVE_Flying);
	}
	else
	{
		// Default falling behavior
		Super::PhysFalling(DeltaTime, Iterations);
		MaxFlySpeed = 600.0f;
		return;
	}

	// Then move the character
	FHitResult Hit;
	

	SafeMoveUpdatedComponent(Velocity * DeltaTime, UpdatedComponent->GetComponentRotation(), true, Hit);
	// Apply gravity again for next frame
	if (!HasValidData())
	{
		return;
	}
	
	Velocity.Z = FMath::Clamp(Velocity.Z, 0, MaxFlySpeed);

}

// Could this be better?
void UCHCharacterMovementComponent::PhysFlying(float deltaTime, int32 Iterations)
{
	Super::PhysFlying(deltaTime, Iterations);
	if (!MyOwner->MyJetpack->IsThrusting())
	{
		BrakingFrictionFactor = 10.0;
		return;
	}
	BrakingFrictionFactor = 2.0;

}
