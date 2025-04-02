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
#include "CHJetpack.h"

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
		// Jumping  &&  Activate Jetpack
		EnhancedInputComponent->BindAction(JumpOrFlyAction, ETriggerEvent::Started, this, &ACherryHillCharacter::StartJumpOrFly);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACherryHillCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACherryHillCharacter::Look);

		// Interact
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
	//	Check if walking on ground to reset values
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		bHasJumped = false;
		bIsFlying = false;
		//bIsThrusting = false;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	//	Jump from ground
	if (!bHasJumped && GetCharacterMovement()->IsMovingOnGround())
	{
		Jump();
		bHasJumped = true;
	}

	//	Check if we have Jetpack Class to advance onto flying
	if (!JetpackClass)
	{
		return;
	}

	//	If in air and not flying yet, activate flying
	else if (bHasJumped && !GetCharacterMovement()->IsMovingOnGround())
	{
		bHasJumped = false;
		bIsFlying = true;
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		UE_LOG(LogTemp, Warning, TEXT("Jetpack engaged!"));

		OnJetpackActivate.Broadcast(this, true);
	}
}

void ACherryHillCharacter::StopJumpOrFly() // More like stop thrusting while flying
{


}


void ACherryHillCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

