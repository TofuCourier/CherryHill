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
				//ThrustAccel = 0.8;
				ThrustInitiate();
				ThrustUp();
			}
			else
			{
				ThrustRelease();
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
//	UE_LOG(LogTemp, Warning, TEXT("BoostSpeed %f"), BoostSpeed);
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
	// bIsJetpackThrusting check may be unnecessary, Keep AttributeComp check? Need to check attribute comp in HasFuel Anyways
	if (!bIsJetpackThrusting || !AttributeComp) return;

	// Check Fuel
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

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Thrust Up
			EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::Started, this, &ACHJetpack::ThrustInitiate);
			EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::Triggered, this, &ACHJetpack::ThrustUp);
			EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::Completed, this, &ACHJetpack::ThrustRelease);

			// Thrust Down
			EnhancedInputComponent->BindAction(ThrustDownAction, ETriggerEvent::Triggered, this, &ACHJetpack::ThrustDown);

			// Thrust Boost
			EnhancedInputComponent->BindAction(ThrustBoostAction, ETriggerEvent::Started, this, &ACHJetpack::ThrustInitiate);
			EnhancedInputComponent->BindAction(ThrustBoostAction, ETriggerEvent::Triggered, this, &ACHJetpack::ThrustBoost);
			EnhancedInputComponent->BindAction(ThrustBoostAction, ETriggerEvent::Completed, this, &ACHJetpack::BoostRelease);

			// No Thrust
			EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::None, this, &ACHJetpack::ThrustToHover);

			// Move
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACHJetpack::Move);

			// Deactivate
			EnhancedInputComponent->BindAction(DeactivateJetpackAction, ETriggerEvent::Completed, this, &ACHJetpack::JetpackDeactivate);

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
		else
		{
			JetpackDeactivate();
		}
	}
}

void ACHJetpack::JetpackDeactivate()
{
	bIsFlying = false;
	bIsThrusting = false;
	bIsStabilizing = false;
	BoostSpeed = 10.0f;
	StabilizeVelocity = 0.0f; // reset
	HoverTime = 0.0f;

	AttributeComp->SetTimerDecay(false);

	
	if (APlayerController* PlayerController = Cast<APlayerController>(OwningCharacter->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FlyMappingContext);
			OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
	}
}

// When character is falling, harder to move due to BoostCharge -- @TODO test this
void ACHJetpack::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	// Set Speed Alpha to a value between 0 and 1 to mirror default speed vs max speed
	float SpeedAlpha = FMath::GetRangePct(BoostSpeedMin, BoostSpeedMax, BoostSpeed);

	// Apply the SpeedAlpha to the Movement Vector. Speed is based off BoostSpeed
	OwningCharacter->AddMovementInput(GetActorForwardVector(), MovementVector.Y * SpeedAlpha);
	OwningCharacter->AddMovementInput(GetActorRightVector(), MovementVector.X * SpeedAlpha);

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

	BoostChargeUp(GetWorld()->GetDeltaSeconds());

	// Interpolate LaunchSpeed toward BoostStrengthMax based on charge
	BoostSpeed = FMath::FInterpTo(BoostSpeed, FMath::Lerp(BoostSpeedMin, BoostSpeedMax, GetBoostChargeAlpha()), GetWorld()->GetDeltaSeconds(), ThrustAccel);


	// Apply upward force via AddMovementInput
	OwningCharacter->AddMovementInput(FVector::UpVector, abs(BoostSpeed * GetWorld()->GetDeltaSeconds()));
	FVector& Velocity = OwningCharacter->GetCharacterMovement()->Velocity;

	UE_LOG(LogTemp, Warning, TEXT("BoostSpeed %f "), BoostSpeed); 
}

void ACHJetpack::ThrustRelease()
{
	// If we aren't boosting, turn off thrust, boost and reset, get ready for Thrust to Hover Bounce
	if (!bIsBoosting)
	{
		bIsStabilizing = true;
		StabilizeVelocity = 0.0f;
		HoverTargetZ = OwningCharacter->GetActorLocation().Z; // or however high you want to hover

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
	}

	// Get boost back up to default if we were just falling - @TODO WIP   final number is threshold
	if (BoostCharge < BoostChargeDefault - 0.3)
	{
		BoostChargeUp(GetWorld()->GetDeltaSeconds());
		return;
	}

	if (BoostCharge > BoostChargeDefault && !IsThrusting())
	{
		BoostChargeDown(GetWorld()->GetDeltaSeconds());
	}


	if (bIsStabilizing)
	{
		FVector Location = OwningCharacter->GetActorLocation();
		float CurrentZ = Location.Z;

		// Spring force toward target
		float Displacement = HoverTargetZ - CurrentZ;

		// Apply spring-damper force: F = kx - cv
		float SpringForce = Displacement * SpringStrength;
		float DampingForce = -StabilizeVelocity * SpringDamping;
		float Accel = SpringForce + DampingForce;

		StabilizeVelocity += Accel * GetWorld()->GetDeltaSeconds();
		StabilizeVelocity = FMath::Clamp(StabilizeVelocity, -BoostSpeedMax, BoostSpeedMax);

		Location.Z += StabilizeVelocity * GetWorld()->GetDeltaSeconds();

		OwningCharacter->SetActorLocation(Location);
		UE_LOG(LogTemp, Warning, TEXT("Displacement %f"), Displacement);
		UE_LOG(LogTemp, Warning, TEXT("StabilizeVelocity %f"), StabilizeVelocity);

		// Stabilize back up if we are falling
		if (Displacement >= 50.0f)
		{
			FVector& Velocity = OwningCharacter->GetCharacterMovement()->Velocity;
			float DesiredLift = FMath::GetMappedRangeValueClamped(FVector2D(100.f, 500.f), FVector2D(300.f, 1500.f), Displacement);

			// Smoothly ramp Z velocity toward DesiredLift
			Velocity.Z = FMath::FInterpTo(Velocity.Z, DesiredLift, GetWorld()->GetDeltaSeconds(), 1.5f);
		}


		// End stabilization once close enough and velocity is low
		// If we were just Boosting Down
		if (Displacement < 100.0f && StabilizeVelocity > 0 && StabilizeVelocity < 150.0f)
		{
			bIsStabilizing = false;
			return;
		}


		// If we were just Boosting Up
		if (FMath::Abs(Displacement) <= 5.0f && StabilizeVelocity <= 20.0f)
		{
 			bIsStabilizing = false;
		}

		// Skip hover bobbing until stabilized
		return;
	}

	if (bIsFlying && !bIsThrusting && !bIsBoosting)
	{
		OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		HoverTime = 0.0f;
		Hover();
	}
}

// @TODO PREVENT HOVER FROM TICKING WHILE THIS IS RUNNING
void ACHJetpack::ThrustDown()
{	
	if (!bIsFlying && bIsThrusting)	return;

	bIsThrusting = false;
	bIsBoosting = false;
	OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);



	bIsStabilizing = true;
	HoverTargetZ = OwningCharacter->GetActorLocation().Z + 50;
	StabilizeVelocity = 0.0f; // reset
	HoverTime = 0.0f;

	// This will effect recover time
	BoostCharge = 0.5f;
}

void ACHJetpack::ThrustBoost()
{
	bIsBoosting = true;

	FVector FinalBoost = FVector::ZeroVector;

	BoostChargeUp(GetWorld()->GetDeltaSeconds());

	// Charge boost towards max
	 BoostSpeed = FMath::FInterpTo(BoostSpeed, FMath::Lerp(BoostSpeedMin, BoostSpeedMax, GetBoostChargeAlpha()), GetWorld()->GetDeltaSeconds(), ThrustAccel);

	// Get the Direction the character is facing
	FVector BoostDirection = OwningCharacter->GetFirstPersonCameraComponent()->GetForwardVector().GetSafeNormal();

	// Multiply speed by direction to give us the Finalboost
	FinalBoost += BoostDirection * BoostSpeed;


	FVector& Velocity = OwningCharacter->GetCharacterMovement()->Velocity;

	// Clamp final velocity to MaxBoostStrength
	Velocity = (Velocity + FinalBoost * GetWorld()->GetDeltaSeconds()).GetClampedToMaxSize(BoostSpeedMax);

}

void ACHJetpack::BoostRelease()
{
	bIsBoosting = false;
	HoverTime = 0.0f;

	if (!bIsThrusting)
	{
		BoostSpeed = SpeedDefault;
		bIsStabilizing = true;
		HoverTargetZ = OwningCharacter->GetActorLocation().Z; // Release height
		StabilizeVelocity = 0.0f;	 // reset

		AttributeComp->IncreaseAttributeDecayValue("Fuel", -1.0f);

	}

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


