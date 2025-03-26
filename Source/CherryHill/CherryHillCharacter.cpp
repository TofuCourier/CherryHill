// Copyright Epic Games, Inc. All Rights Reserved.

#include "CherryHillCharacter.h"
#include "CherryHillProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "CHInteractionComponent.h"
#include "CHActionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ACherryHillCharacter

ACherryHillCharacter::ACherryHillCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	InteractionComp = CreateDefaultSubobject<UCHInteractionComponent>(TEXT("InterationComp"));

	ActionComp = CreateDefaultSubobject<UCHActionComponent>(TEXT("ActionComp"));

}

//////////////////////////////////////////////////////////////////////////// Input

void ACherryHillCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ACherryHillCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpOrFlyAction, ETriggerEvent::Started, this, &ACherryHillCharacter::StartJumpOrFly);
		//EnhancedInputComponent->BindAction(JumpOrFlyAction, ETriggerEvent::Started, this, &ACherryHillCharacter::StopJumpOrFly);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACherryHillCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACherryHillCharacter::Look);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ACherryHillCharacter::Interact);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void ACherryHillCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ACherryHillCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACherryHillCharacter::Interact(const FInputActionValue& Value)
{
	if (InteractionComp)
	{
		InteractionComp->PrimaryInteract();
	}
}

void ACherryHillCharacter::StartJumpOrFly()
{
	// Step 1: Jump from ground
	if (!bHasJumped && !bIsFlying)
	{
		Jump();
		bHasJumped = true;
	}
	// Step 2: If in air and not flying yet, activate flying and thrust
	else if (bHasJumped && !bIsFlying)
	{
		bHasJumped = false;
		bIsFlying = true;
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		CurrentThrust = 0.f;
		UE_LOG(LogTemp, Warning, TEXT("Jetpack engaged!"));

		if (!bBoundAction)
		{
			if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(GetController()->InputComponent))
			{

				// Fire
				EnhancedInputComponent->BindAction(JumpOrFlyAction, ETriggerEvent::Triggered, this, &ACherryHillCharacter::JetPackThrust);
				bBoundAction = true;
				EnhancedInputComponent->BindAction(JumpOrFlyAction, ETriggerEvent::Completed, this, &ACherryHillCharacter::StopJumpOrFly);
				JetPackThrust();
			}
		}
	}
	// Step 3: If already flying, start thrusting up
	//else if (bIsFlying)
	//{
	//	JetPackThrust();
	//}
}

void ACherryHillCharacter::StopJumpOrFly() // More like stop thrusting while flying
{
	if (bIsThrusting)
	{
  		bIsThrusting = false;
		UE_LOG(LogTemp, Warning, TEXT("stopped Thrusting"));
		bWasThrustingLastFrame = true;
	}

}

void ACherryHillCharacter::JetPackThrust()
{
	ActionComp->StartActionByName(this, "Thrust");
	bIsThrusting = true;
	
}

void ACherryHillCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);


	// Reset jump if we land
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		bHasJumped = false;
		bIsFlying = false;
		bIsThrusting = false;
		CurrentThrust = 0.f;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	// Give a small upward bounce
	if (bWasThrustingLastFrame && !bIsThrusting)
	{
		FVector CurrentVelocity = GetCharacterMovement()->Velocity;
		CurrentVelocity.Z = FMath::Clamp(CurrentVelocity.Z, CurrentVelocity.Z, MaxThrust);
		GetCharacterMovement()->Velocity = CurrentVelocity;

		UE_LOG(LogTemp, Warning, TEXT("Jetpack bounce!"));
		bWasThrustingLastFrame = false;
	}

	// Slowly decrease vertical velocity (simulate light descent)
	if (bIsFlying && !bIsThrusting)
	{
		FVector CurrentVelocity = GetCharacterMovement()->Velocity;

		float FallRate = -40.0f; // adjust for how quickly they descend

		float NewVelocity = FallRate * DeltaSeconds;

		CurrentVelocity.Z += NewVelocity;
		CurrentVelocity.Z = FMath::Clamp(CurrentVelocity.Z, FallRate, MaxThrust);

		GetCharacterMovement()->Velocity = CurrentVelocity;
		UE_LOG(LogTemp, Warning, TEXT("velocity = %f"), CurrentVelocity.Z);
	}


}

