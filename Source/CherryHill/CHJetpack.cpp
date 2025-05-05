// Fill out your copyright notice in the Description page of Project Settings.


#include "CHJetpack.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "CherryHillCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "CHAttributeComponent.h"
#include "TimerManager.h"

ACHJetpack::ACHJetpack()
{
	PrimaryActorTick.bCanEverTick = true;

	AttributeComp = CreateDefaultSubobject<UCHAttributeComponent>(TEXT("AttributeComp"));

	if (!AttributeComp->Attributes.Contains("Fuel"))
	{
		AttributeComp->AddAttribute("Fuel", 500.0f, 500.0f, -1.0f);
	}

}

void ACHJetpack::BeginPlay()
{
	Super::BeginPlay();

}

// @TODO FIX THIS TO NOT HAVE A HARD CODED INPUT KEY, but also there has to be a better way to do this?
void ACHJetpack::OnActivateHeldTimer(APlayerController* Controller)
{
	FTimerDelegate InputCheckDelegate;
	InputCheckDelegate.BindLambda([this, Controller]()
		{
			if (Controller->IsInputKeyDown(EKeys::Gamepad_RightShoulder) || Controller->IsInputKeyDown(EKeys::SpaceBar))
			{
				ThrustAccel = 0.8f;
				InitiateAscend();
				Ascend();
			}
			else
			{
				AscendComplete();
				GetWorld()->GetTimerManager().ClearTimer(InputCheckDelayHandle);
			}
		});

		GetWorld()->GetTimerManager().SetTimer(
		InputCheckDelayHandle,
		InputCheckDelegate,
		GetWorld()->GetDeltaSeconds(),
		true
	);

 }

void ACHJetpack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Warning, TEXT("isflying: %s"), bIsFlying ? TEXT("true") : TEXT("false"));
}

void ACHJetpack::AttachJetpack(ACharacter* TargetCharacter)
{	
	OwningCharacter = Cast<ACherryHillCharacter>(TargetCharacter);

	if (!OwningCharacter) return;

	if (GetClass()->IsChildOf(ACHJetpack::StaticClass()))
	{
		this->AttachToActor(TargetCharacter, FAttachmentTransformRules::KeepRelativeTransform);
		OwningCharacter->OnJetpackActivated.AddDynamic(this, &ACHJetpack::OnJetpackActivated);
		OwningCharacter->MyJetpack = this;
	}
}



void ACHJetpack::OnJetpackActivated_Implementation(AActor* IntigatorActor, ACHJetpack* Jetpack, bool bActivated = true)
{
	// Also checks for attribute comp on Jetpack (needed for fuel)
	if (!HasFuel())
	{
		OnJetpackDeactivated();
		return;
	}

	// Bind mappings and inputs, then set flying values  
	if (APlayerController* PlayerController = Cast<APlayerController>(OwningCharacter->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(FlyMappingContext, 1);
		}

		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
		if (!EnhancedInputComponent)
		{
			OnJetpackDeactivated();
			return;
		}

		// If Jetpack inputs are already bound, skip.
		if (!bJetpackInputBound)
		{
			// Thrust Up
			EnhancedInputComponent->BindAction(AscendAction, ETriggerEvent::Started, this, &ACHJetpack::InitiateAscend);
			EnhancedInputComponent->BindAction(AscendAction, ETriggerEvent::Triggered, this, &ACHJetpack::Ascend);
			EnhancedInputComponent->BindAction(AscendAction, ETriggerEvent::Completed, this, &ACHJetpack::AscendComplete);

			// Thrust Down
			EnhancedInputComponent->BindAction(DescendAction, ETriggerEvent::Started, this, &ACHJetpack::InitiateDescend);
			EnhancedInputComponent->BindAction(DescendAction, ETriggerEvent::Triggered, this, &ACHJetpack::Descend);
			EnhancedInputComponent->BindAction(DescendAction, ETriggerEvent::Completed, this, &ACHJetpack::DescendComplete);

			// Thrust Boost
			EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Started, this, &ACHJetpack::BoostInitiate);
			EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Triggered, this, &ACHJetpack::Boost);
			EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Completed, this, &ACHJetpack::BoostComplete);

			// No Thrust
			EnhancedInputComponent->BindAction(AscendAction, ETriggerEvent::None, this, &ACHJetpack::TransitionToHover);

			// Move
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACHJetpack::Move);

			// Deactivate
			EnhancedInputComponent->BindAction(DeactivateJetpackAction, ETriggerEvent::Completed, this, &ACHJetpack::OnJetpackDeactivated);
			bJetpackInputBound = true;
		}
			
		// Fuel Decay On
		AttributeComp->SetTimerDecay(true);

		// ** Set Default Speed  **
		// BoostSpeedMax = BoostSpeedMax - Mass;   // Uncomment when adding in mass. Mass effects velocity.------- Will need to rework this, might not clamp to max when boosting.
		bIsFlying = true;
		SpeedDefault = (BoostSpeedMax / 2.0f);
		BoostSpeed = SpeedDefault;
		BoostCharge = 0.5;
		OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		bIsStabilizing = true;

		// Allows for continuous ascent from activation jump
		OnActivateHeldTimer(PlayerController);
	}
	return;
}

void ACHJetpack::OnJetpackDeactivated_Implementation()
{
	// Set to falling and all flying checks are false
	OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	bIsFlying = false;
	bIsAscending = false;
	bIsBoosting = false;
	bIsStabilizing = false;
	BoostCharge = 0.0f;

	AttributeComp->SetTimerDecay(false);

	// Remove flying input mapping from controller
	if (APlayerController* PlayerController = Cast<APlayerController>(OwningCharacter->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FlyMappingContext);
		}
	}
}

void ACHJetpack::Move(const FInputActionValue& Value)
{
 	FVector2D MovementVector = Value.Get<FVector2D>();

	// SpeedAlpha is based of BoostCharge %  // @TODO fix the boost speed. It doesn't properly reflect current speed.
	float SpeedAlpha = FMath::GetRangePct(0.0f, BoostSpeedMax, BoostSpeed);

	// Prevents any vertical movement input from sneaking in due to camera pitch or character tilt
	FVector FlatForward = GetActorForwardVector();
	FlatForward.Z = 0.0f;
	FlatForward.Normalize();

	FVector FlatRight = GetActorRightVector();
	FlatRight.Z = 0.0f;
	FlatRight.Normalize();

	float MovementDampen = bIsStabilizing ? 1.0f : 1.0f;
	// Apply movement input with flattened vectors
	OwningCharacter->AddMovementInput(FlatForward, MovementVector.Y * SpeedAlpha * MovementDampen);
	OwningCharacter->AddMovementInput(FlatRight, MovementVector.X * SpeedAlpha * MovementDampen);
}

void ACHJetpack::InitiateAscend()
{
	bIsAscending = true;
	if (!HasFuel())
	{
		OnJetpackDeactivated();
		return;
	}
	AttributeComp->IncreaseAttributeDecayValue("Fuel", -5.0f);
}

void ACHJetpack::Ascend()
{
	if (!bIsFlying || IsDescending()) return;

	// If already boosting, we do not want to double the charge
	if (!bIsBoosting)
	{
		BoostChargeUp(GetWorld()->GetDeltaSeconds());
	}

	FVector CurrentVelocity = OwningCharacter->GetVelocity();
	SavedLateralVelocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);

	// Target upward speed based on charge
	float TargetThrust = FMath::Lerp(BoostSpeed, BoostSpeedMax, GetBoostChargeAlpha());

	// Smooth acceleration toward target
	BoostSpeed = FMath::FInterpTo(BoostSpeed, TargetThrust, GetWorld()->GetDeltaSeconds(), ThrustAccel);

	// Compute upward acceleration for this frame
	float ThrustAcceleration = BoostSpeed * GetWorld()->GetDeltaSeconds();

	// Apply that as a Z boost ADDITIVE to current velocity
	FVector& Velocity = OwningCharacter->GetCharacterMovement()->Velocity;
	Velocity.Z += ThrustAcceleration;


	// Optional: clamp final Z velocity to prevent escape speed
	Velocity.Z = FMath::Clamp(Velocity.Z, -BoostSpeedMax, BoostSpeedMax);
	OwningCharacter->GetCharacterMovement()->Velocity.X = SavedLateralVelocity.X;
	OwningCharacter->GetCharacterMovement()->Velocity.Y = SavedLateralVelocity.Y;
}

void ACHJetpack::AscendComplete()
{
	// If we aren't boosting, turn off thrust, boost and reset, get ready for Thrust to Hover Bounce
	if (!bIsBoosting)
	{
		bIsStabilizing = true;
		HoverTargetZ = OwningCharacter->GetActorLocation().Z; // or however high you want to hover. Too high of a number will prevent bounce from occuring

		BoostSpeed = SpeedDefault;

		AttributeComp->IncreaseAttributeDecayValue("Fuel", -1.0f);
	}
		bIsAscending = false;
		HoverTime = 0.0f;
}

void ACHJetpack::TransitionToHover()
{
	float StabilizeVelocity = 0.0f;
	if (!HasFuel())
	{
		OnJetpackDeactivated();
		return;
	}
	if (OwningCharacter->GetCharacterMovement()->IsMovingOnGround())
	{
		bIsStabilizing = false;
		OnJetpackDeactivated();
		return;
	}

	// Get boost back up to default if we were just falling - @TODO WIP   final number is threshold
	if (BoostCharge < BoostChargeDefault - 0.05)
	{
		BoostChargeUp(GetWorld()->GetDeltaSeconds());
		BoostSpeed = FMath::FInterpTo(BoostSpeed, GetBoostChargeAlpha(), GetWorld()->GetDeltaSeconds(), ThrustAccel);
	}

	if (BoostCharge > BoostChargeDefault && !IsLaunching())
	{
		BoostChargeDown(GetWorld()->GetDeltaSeconds());
		BoostSpeed = FMath::FInterpTo(BoostSpeed, GetBoostChargeAlpha(), GetWorld()->GetDeltaSeconds(), ThrustAccel);
	}

	// I still need to fix the slowdown when we stabilize
	if (bIsStabilizing && !IsLaunching() && !bIsDescending)
	{
		FVector& Velocity = OwningCharacter->GetCharacterMovement()->Velocity;
		SavedLateralVelocity.X = Velocity.X;
		SavedLateralVelocity.Y = Velocity.Y;
		const float DeltaTime = GetWorld()->GetDeltaSeconds();
		const float Displacement = HoverTargetZ - OwningCharacter->GetActorLocation().Z;

		// Spring-damper logic
		const float SpringForce = Displacement * SpringStrength;
		const float DampingForce = -StabilizeVelocity * SpringDamping;
		const float Accel = SpringForce + DampingForce;

		StabilizeVelocity += Accel * DeltaTime;

		// Clamp to BoostCharge-influenced thrust
		const float TargetThrust = FMath::Lerp(BoostSpeedMin, BoostSpeedMax, GetBoostChargeAlpha());
		StabilizeVelocity = FMath::Clamp(StabilizeVelocity, -TargetThrust, TargetThrust);

		// Rescue lift if falling too fast
		if (Velocity.Z < -5.0f)
		{
			const float BoostRescue = FMath::Lerp(BoostSpeedMin, BoostSpeedMax, GetBoostChargeAlpha());
			Velocity.Z = FMath::FInterpTo(Velocity.Z, BoostRescue, DeltaTime, 4.0f);
		}
		else
		{
			Velocity.Z = FMath::FInterpTo(Velocity.Z, StabilizeVelocity, DeltaTime, 2.0f);
		}

		// --- Forward momentum restore ---
		FVector InputVector = OwningCharacter->GetCharacterMovement()->GetLastInputVector();
		InputVector.Z = 0.0f;

		const float DesiredSpeed = FMath::Lerp(SpeedDefault, BoostSpeedMax, GetBoostChargeAlpha());
		if (!InputVector.IsNearlyZero())
		{
			// Player is giving input — apply full speed instantly
			InputVector.Normalize();
			Velocity.X = InputVector.X * DesiredSpeed;
			Velocity.Y = InputVector.Y * DesiredSpeed;
		}
		else
		{
			// No input — restore previous movement gently
			Velocity.X = FMath::FInterpTo(Velocity.X, SavedLateralVelocity.X, DeltaTime, 4.0f);
			Velocity.Y = FMath::FInterpTo(Velocity.Y, SavedLateralVelocity.Y, DeltaTime, 4.0f);
		}

		// Finish stabilization check
		const bool CloseVel = FMath::IsNearlyEqual(Velocity.Z, 0.0, 10.0f);
		const bool CalmSpring = FMath::IsNearlyEqual(StabilizeVelocity, PrevStabilizeVelocity, 5.0f);
		const bool BoostIsStable = FMath::IsNearlyEqual(BoostCharge, BoostChargeDefault, 0.05f);

		// Update our speed
		BoostSpeed = DesiredSpeed;

		// If stabilized, move on to hover
		if (BoostIsStable && CalmSpring && CloseVel)
		{
			bIsStabilizing = false;
			HoverTime = 0.0f;
			OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);

		}

		// Track for next frame
		PrevStabilizeVelocity = StabilizeVelocity;
		return;
	}
	

	// Before we start hover, we cant be thrusting.
	if (bIsFlying && !IsLaunching() && !IsDescending())
	{
		Hover();
	}
}

void ACHJetpack::InitiateDescend()
{
	bIsAscending = false;
	bIsDescending = true;
	FVector CurrentVelocity = OwningCharacter->GetVelocity();
	SavedLateralVelocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);
}

void ACHJetpack::Descend()
{	
	if (!bIsFlying && IsLaunching()) return;

	// Smooth velocity transition right after turning down thrusters
	OwningCharacter->GetCharacterMovement()->Velocity.X = SavedLateralVelocity.X;
	OwningCharacter->GetCharacterMovement()->Velocity.Y = SavedLateralVelocity.Y;

	// We are falling technically
	OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	
	// Keep some BoostCharge for when we hover again
	BoostCharge = 0.5;
}

void ACHJetpack::DescendComplete()
{
	bIsDescending = false;
	bIsStabilizing = true;
	HoverTargetZ = OwningCharacter->GetActorLocation().Z;
}

void ACHJetpack::BoostInitiate()
{
	bIsBoosting = true;
	if (!HasFuel())
	{
		OnJetpackDeactivated();
		return;
	}
	AttributeComp->IncreaseAttributeDecayValue("Fuel", -5.0f);
}

void ACHJetpack::Boost()
{
	if (!bIsFlying) return;

	//FVector FinalBoost = FVector::ZeroVector;

	// Charge boost towards max
	BoostChargeUp(GetWorld()->GetDeltaSeconds());

	// Increase and Clamp speed based off Boost Charge
	BoostSpeed = FMath::FInterpTo(BoostSpeed, FMath::Lerp(BoostSpeedMin, BoostSpeedMax, GetBoostChargeAlpha()), GetWorld()->GetDeltaSeconds(), ThrustAccel);

	// Get the Direction the character is facing
	FVector BoostDirection = OwningCharacter->GetFirstPersonCameraComponent()->GetForwardVector().GetSafeNormal();

	// Multiply speed by direction to give us the Final boost
	FVector FinalBoost = BoostDirection * BoostSpeed;

	// Add boost and clamp Characters final velocity to MaxBoostStrength
	FVector& Velocity = OwningCharacter->GetCharacterMovement()->Velocity;
	Velocity = (Velocity + FinalBoost * GetWorld()->GetDeltaSeconds());//.GetClampedToMaxSize(BoostSpeedMax);
}

void ACHJetpack::BoostComplete()
{
	// If we are thrusting already, we don't want to override these values
	if (!bIsAscending)
	{
		bIsStabilizing = true;
		HoverTargetZ = OwningCharacter->GetActorLocation().Z;

		AttributeComp->IncreaseAttributeDecayValue("Fuel", -1.0f);
		HoverTime = 0.0f;
	}
	bIsBoosting = false;
}

void ACHJetpack::Hover()
{
	// Calculate bobbing offset
	HoverTime += GetWorld()->GetDeltaSeconds();
	float BobOffset = FMath::Sin(HoverTime * HoverFrequency) * HoverAmplitude;

	// Apply bobbing to velocity
	FVector Velocity = OwningCharacter->GetVelocity();
	Velocity.Z = BobOffset;
	OwningCharacter->GetCharacterMovement()->Velocity = Velocity;
}

float ACHJetpack::GetCurrentFuel()
{
	if (!AttributeComp)
	{
		OnJetpackDeactivated();
		return false;
	}

	for (TPair<FName, FAttribute>& AttributePair : AttributeComp->Attributes)
	{
		FName Name = AttributePair.Key;

		if (Name == "Fuel")
		{
			FAttribute& Attribute = AttributePair.Value;

			return Attribute.CurrentValue;
		}
	}
	return false;
}

bool ACHJetpack::HasFuel()
{
	if (GetCurrentFuel() <= 0.0f)
	{
		return false;
	}
	return true;
}

void ACHJetpack::BoostChargeUp(float DeltaTime)
{
	BoostCharge += DeltaTime * ThrustAccel;
	BoostCharge = FMath::Clamp(BoostCharge, 0.0f, BoostChargeMax);
	AttributeComp->OnAttributeChange.Broadcast(this, AttributeComp);
}

void ACHJetpack::BoostChargeDown(float DeltaTime)
{
	// The 1 is just a temp value 
	BoostCharge -= DeltaTime * 2;
	BoostCharge = FMath::Clamp(BoostCharge, 0.0f, BoostChargeMax);
	AttributeComp->OnAttributeChange.Broadcast(this, AttributeComp);
}

float ACHJetpack::GetBoostChargeAlpha() const
{
	return (BoostChargeMax > 0.0f) ? BoostCharge / BoostChargeMax : 0.0f;
}


