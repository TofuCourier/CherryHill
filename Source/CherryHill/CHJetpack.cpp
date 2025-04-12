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
}

void ACHJetpack::BeginPlay()
{
	Super::BeginPlay();

	if (!AttributeComp->Attributes.Contains("Fuel"))
	{
		AttributeComp->AddAttribute("Fuel", 1000.0f, 1000.0f, -1.0f);
	}
	
}

// @TODO FIX THIS TO NOT BE HARD CODED SPACE BAR
void ACHJetpack::OnActivateThrustTimer(APlayerController* Controller)
{
	FTimerDelegate InputCheckDelegate;
	InputCheckDelegate.BindLambda([this, Controller]()
		{
			if (Controller->IsInputKeyDown(EKeys::SpaceBar))
			{
				ThrustAccel = 1.0;
				ThrustInitiate();
				ThrustUp();
			}
			else
			{
				ThrustUpComplete();
				GetWorld()->GetTimerManager().ClearTimer(InputCheckDelayHandle);
			}
		});

		GetWorld()->GetTimerManager().SetTimer(
		InputCheckDelayHandle,
		InputCheckDelegate,
		0.02f,
		true
	);

 }

void ACHJetpack::ThrustInitiate()
{
	if (!HasFuel())
	{
		JetpackDeactivate();
		return;
	}
	AttributeComp->IncreaseAttributeDecayValue("Fuel", -5.0f);


}

void ACHJetpack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACHJetpack::AttachJetpack(ACharacter* TargetCharacter)
{	
	OwningCharacter = Cast<ACherryHillCharacter>(TargetCharacter);

	if (!OwningCharacter) return;

	if (GetClass()->IsChildOf(ACHJetpack::StaticClass()))
	{
		this->AttachToActor(TargetCharacter, FAttachmentTransformRules::KeepRelativeTransform);
		OwningCharacter->OnJetpackActivate.AddDynamic(this, &ACHJetpack::OnJetpackActivate);
		OwningCharacter->MyJetpack = this;
	}
}

void ACHJetpack::OnJetpackActivate(AActor* IntigatorActor, ACHJetpack* Jetpack, bool bIsJetpackThrusting)
{
	// Also checks for attribute comp
	if (!HasFuel())
	{
		JetpackDeactivate();
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(OwningCharacter->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(FlyMappingContext, 1);
		}

		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);

		if (!EnhancedInputComponent)
		{
			JetpackDeactivate();
			return;
		}

		if (!bJetpackInputBound)
		{
			// Thrust Up
			EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::Started, this, &ACHJetpack::ThrustInitiate);
			EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::Triggered, this, &ACHJetpack::ThrustUp);
			EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::Completed, this, &ACHJetpack::ThrustUpComplete);

			// Thrust Down
			EnhancedInputComponent->BindAction(ThrustDownAction, ETriggerEvent::Started, this, &ACHJetpack::ThrustDownInitiate);
			EnhancedInputComponent->BindAction(ThrustDownAction, ETriggerEvent::Triggered, this, &ACHJetpack::ThrustDown);
			EnhancedInputComponent->BindAction(ThrustDownAction, ETriggerEvent::Completed, this, &ACHJetpack::ThrustDownComplete);

			// Thrust Boost
			EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Started, this, &ACHJetpack::ThrustInitiate);
			EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Triggered, this, &ACHJetpack::Boost);
			EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Completed, this, &ACHJetpack::BoostComplete);

			// No Thrust
			EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::None, this, &ACHJetpack::ThrustToHover);

			// Move
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACHJetpack::Move);

			// Deactivate
			EnhancedInputComponent->BindAction(DeactivateJetpackAction, ETriggerEvent::Completed, this, &ACHJetpack::JetpackDeactivate);
			bJetpackInputBound = true;
		}
			
		// Fuel Decay On
		AttributeComp->SetTimerDecay(true);

		// ** Set Default Speed  **
		// BoostSpeedMax = BoostSpeedMax - Mass;   // Uncomment when adding in mass. Mass effects velocity.
		SpeedDefault = (BoostSpeedMax / 2.0f);

		bIsFlying = true;
		OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		
		// Allows for continuous thrust from activation jump
		OnActivateThrustTimer(PlayerController);
	}
}

void ACHJetpack::JetpackDeactivate()
{
	bIsFlying = false;
	bIsThrusting = false;
	bIsStabilizing = false;
	BoostSpeed = 0.0f;
	StabilizeVelocity = 0.0f; // reset
	HoverTime = 0.0f;
	BoostCharge = 0.0f;

	AttributeComp->SetTimerDecay(false);

	if (APlayerController* PlayerController = Cast<APlayerController>(OwningCharacter->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FlyMappingContext);
			OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
	}
}

void ACHJetpack::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	// SpeedAlpha is based of BoostCharge %  // @TODO fix the boost speed. It doesnt properly reflect current speed.
	float SpeedAlpha = FMath::GetRangePct(BoostSpeedMin, BoostSpeedMax, BoostSpeed);

	// Prevents any vertical movement input from sneaking in due to camera pitch or character tilt
	FVector FlatForward = GetActorForwardVector();
	FlatForward.Z = 0.0f;
	FlatForward.Normalize();

	FVector FlatRight = GetActorRightVector();
	FlatRight.Z = 0.0f;
	FlatRight.Normalize();

	float MovementDampen = bIsStabilizing ? 0.9f : 1.0f;
	// Apply movement input with flattened vectors
	OwningCharacter->AddMovementInput(FlatForward, MovementVector.Y * SpeedAlpha * MovementDampen);
	OwningCharacter->AddMovementInput(FlatRight, MovementVector.X * SpeedAlpha * MovementDampen);
}

bool ACHJetpack::IsThrusting()
{
	if (bIsThrusting || bIsBoosting)
	{
		return true;

	}
	return false;
}

void ACHJetpack::ThrustUp()
{
	if (!bIsFlying) return;
	bIsThrusting = true;

	const float DeltaTime = GetWorld()->GetDeltaSeconds();


	// If already boosting, we do not want to double the charge
	if (!bIsBoosting)
	{
		BoostChargeUp(GetWorld()->GetDeltaSeconds());
	}

	// Target upward speed based on charge
	float TargetThrust = FMath::Lerp(BoostSpeedMin, BoostSpeedMax, GetBoostChargeAlpha());

	// Smooth acceleration toward target
	BoostSpeed = FMath::FInterpTo(BoostSpeed, TargetThrust, GetWorld()->GetDeltaSeconds(), ThrustAccel);

	// Compute upward acceleration for this frame
	float ThrustAcceleration = BoostSpeed * GetWorld()->GetDeltaSeconds();

	// Apply that as a Z boost ADDITIVE to current velocity
	FVector& Velocity = OwningCharacter->GetCharacterMovement()->Velocity;
	Velocity.Z += ThrustAcceleration;

	// Optional: clamp final Z velocity to prevent escape speed
	Velocity.Z = FMath::Clamp(Velocity.Z, -BoostSpeedMax, BoostSpeedMax);
}

void ACHJetpack::ThrustUpComplete()
{
	// If we aren't boosting, turn off thrust, boost and reset, get ready for Thrust to Hover Bounce
	if (!bIsBoosting)
	{
		bIsStabilizing = true;
		StabilizeVelocity = 0.0f;
		HoverTargetZ = OwningCharacter->GetActorLocation().Z; // or however high you want to hover. Too high of a number will prevent bounce from occuring

		BoostSpeed = SpeedDefault;

		AttributeComp->IncreaseAttributeDecayValue("Fuel", -1.0f);
	}
		bIsThrusting = false;
		HoverTime = 0.0f;
}

void ACHJetpack::ThrustToHover()
{
	if (!HasFuel())
	{
		JetpackDeactivate();
		return;
	}

	if (OwningCharacter->GetCharacterMovement()->IsMovingOnGround())
	{
		bIsStabilizing = false;
		JetpackDeactivate();
		return;
	}

	// Get boost back up to default if we were just falling - @TODO WIP   final number is threshold
	if (BoostCharge < BoostChargeDefault - 0.05)
	{
		BoostChargeUp(GetWorld()->GetDeltaSeconds());
	}

	if (BoostCharge > BoostChargeDefault && !IsThrusting())
	{
		BoostChargeDown(GetWorld()->GetDeltaSeconds());
	}

	// I still need to fix the slowdown when we stabilize
	if (bIsStabilizing && !IsThrusting() && !bInThrustFall)
	{
		FVector& Velocity = OwningCharacter->GetCharacterMovement()->Velocity;
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
		if (Velocity.Z < -200.f)
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
		const bool CloseZ = FMath::IsNearlyEqual(Displacement, PrevDisplacement, 1.0f);
		const bool CalmSpring = FMath::Abs(StabilizeVelocity) <= PrevStabilizeVelocity;
		const bool BoostIsStable = FMath::IsNearlyEqual(BoostCharge, BoostChargeDefault, 0.05f);

		if (CloseZ && CalmSpring && BoostIsStable)
		{
			bIsStabilizing = false;
			bWithinStableRange = true;
			HoverTime = 0.0f;
		}

		// Track for next frame
		PrevStabilizeVelocity = StabilizeVelocity;
		PrevDisplacement = Displacement;

		return;
	}

	if (bIsFlying && !IsThrusting() && !bInThrustFall)
	{
		// We are hovering, so we are flying
		OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		Hover();
	}
}

void ACHJetpack::ThrustDownInitiate()
{
	bIsThrusting = false;
	bInThrustFall = true;
	FVector CurrentVelocity = OwningCharacter->GetVelocity();
	SavedLateralVelocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);

}

// @TODO PREVENT HOVER FROM TICKING WHILE THIS IS RUNNING
void ACHJetpack::ThrustDown()
{	
	if (!bIsFlying && bIsThrusting)	return;

	OwningCharacter->GetCharacterMovement()->Velocity.X = SavedLateralVelocity.X ;
	OwningCharacter->GetCharacterMovement()->Velocity.Y = SavedLateralVelocity.Y;
	OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);

	BoostCharge = 0.5;
}

void ACHJetpack::ThrustDownComplete()
{
	bInThrustFall = false;
	bIsStabilizing = true;
	HoverTargetZ = OwningCharacter->GetActorLocation().Z;
	StabilizeVelocity = 0.0f; 
	BoostChargeUp(GetWorld()->GetDeltaSeconds());
}

void ACHJetpack::Boost()
{
	bIsBoosting = true;

	FVector FinalBoost = FVector::ZeroVector;

	// Charge boost towards max
	BoostChargeUp(GetWorld()->GetDeltaSeconds());

	// Increase and Clamp speed based off Boost Charge
	BoostSpeed = FMath::FInterpTo(BoostSpeed, FMath::Lerp(BoostSpeedMin, BoostSpeedMax, GetBoostChargeAlpha()), GetWorld()->GetDeltaSeconds(), ThrustAccel);

	// Get the Direction the character is facing
	FVector BoostDirection = OwningCharacter->GetFirstPersonCameraComponent()->GetForwardVector().GetSafeNormal();

	// Multiply speed by direction to give us the Final boost
	FinalBoost += BoostDirection * BoostSpeed;

	// Add boost and clamp Characters final velocity to MaxBoostStrength
	FVector& Velocity = OwningCharacter->GetCharacterMovement()->Velocity;
	Velocity = (Velocity + FinalBoost * GetWorld()->GetDeltaSeconds()).GetClampedToMaxSize(BoostSpeedMax);
}

void ACHJetpack::BoostComplete()
{
	if (!bIsThrusting)
	{
		BoostSpeed = SpeedDefault;
		bIsStabilizing = true;
		HoverTargetZ = OwningCharacter->GetActorLocation().Z; // Release height
		StabilizeVelocity = 0.0f;	 // reset

		AttributeComp->IncreaseAttributeDecayValue("Fuel", -1.0f);
		HoverTime = 0.0f;
	}
	bIsBoosting = false;
}

void ACHJetpack::Hover()
{
	HoverTime += GetWorld()->GetDeltaSeconds();

	// Calculate bobbing offset
	float BobOffset = FMath::Sin(HoverTime * HoverFrequency) * HoverAmplitude;

	// Apply bobbing to velocity
	FVector Velocity = OwningCharacter->GetVelocity();
	Velocity.Z = BobOffset;
	OwningCharacter->GetCharacterMovement()->Velocity = Velocity;
	StabilizeVelocity = 1.0f;
}

float ACHJetpack::GetCurrentFuel()
{
	if (!AttributeComp)
	{
		JetpackDeactivate();
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
	BoostCharge -= DeltaTime * 1;
	BoostCharge = FMath::Clamp(BoostCharge, 0.0f, BoostChargeMax);
	AttributeComp->OnAttributeChange.Broadcast(this, AttributeComp);
}

float ACHJetpack::GetBoostChargeAlpha() const
{
	return (BoostChargeMax > 0.0f) ? BoostCharge / BoostChargeMax : 0.0f;
}


