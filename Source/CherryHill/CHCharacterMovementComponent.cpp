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
	// If Jetpack is on, character is flying
 	if (MyOwner->IsJetpackActive() && MyOwner->MyJetpack->IsLaunching() && MyOwner->MyJetpack->IsFlying())
	{
		SetMovementMode(MOVE_Flying);
	}
	else
	{
		// Default falling behavior
 		Super::PhysFalling(DeltaTime, Iterations);
		return;
	}


	////// Smooth it out ///////////////////////////////////
	
	// Then move the character
	FHitResult Hit;

	SafeMoveUpdatedComponent(Velocity * DeltaTime, UpdatedComponent->GetComponentRotation(), true, Hit);

	// Apply gravity again for next frame
	if (!HasValidData())
	{
		return;
	}
	
	Velocity.Z = FMath::Clamp(Velocity.Z, 0, MaxFlySpeed);

	UE_LOG(LogTemp, Warning, TEXT("Velocity"), Velocity.Z);

}

// Could this be better?
void UCHCharacterMovementComponent::PhysFlying(float deltaTime, int32 Iterations)
{
	Super::PhysFlying(deltaTime, Iterations);
	if (!MyOwner->MyJetpack->IsLaunching())
	{
		BrakingFrictionFactor = 10.0;
		return;
	}
	BrakingFrictionFactor = 2.0;

}
