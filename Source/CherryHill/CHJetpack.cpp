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

// Sets default values
ACHJetpack::ACHJetpack()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AttributeComp = CreateDefaultSubobject<UCHAttributeComponent>(TEXT("AttributeComp"));
}

// Called when the game starts or when spawned
void ACHJetpack::BeginPlay()
{
	Super::BeginPlay();

	if (!AttributeComp->Attributes.Contains("Fuel"))
	{
		AttributeComp->AddAttribute("Fuel", 1000.0f, 1000.0f, -1.0f);
	}
	
}

void ACHJetpack::OnActivateThrustTimer(APlayerController* Controller)
{
	FTimerHandle InputCheckDelayHandle;

	GetWorld()->GetTimerManager().SetTimer(
		InputCheckDelayHandle,
		[this, Controller, &InputCheckDelayHandle]()
		{
			if (Controller->IsInputKeyDown(EKeys::SpaceBar))
			{
				// Simulate thrust while holding space
				ThrustInitiate();
				ThrustUp();
			}
			else
			{
				//GetWorld()->GetTimerManager().ClearTimer(InputCheckDelayHandle);
				//GetWorld()->GetTimerManager().PauseTimer(InputCheckDelayHandle);
				//ThrustRelease();
			}
		},
		0.02,  // Not DeltaSeconds to prevent jitter on frame rate change
		true   
	);
	return;
}


void ACHJetpack::ThrustInitiate()
{
	if (!HasFuel())
	{
		JetpackDeactivate();
		return;
	}
	AttributeComp->IncreaseAttributeDecayValue("Fuel", -4.0f);


}

// Called every frame
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
	// This first check may be unnecessary
	if (!bIsJetpackThrusting) return;

	// Check Fuel
	if (!HasFuel())
	{
		JetpackDeactivate();
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(IntigatorActor->GetInstigatorController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the thrust action when using touch input
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

			// Deactivate
			EnhancedInputComponent->BindAction(DeactivateJetpackAction, ETriggerEvent::Completed, this, &ACHJetpack::JetpackDeactivate);

			OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);

			bIsFlying = true;

			// Continuous thrust from activation
			OnActivateThrustTimer(PlayerController);

			// Fuel Decay On
			AttributeComp->SetTimerDecayActive(true);
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
	LaunchSpeed = 0.0f;
	StabilizeVelocity = 0.0f; // reset
	HoverTime = 0.0f;

	AttributeComp->SetTimerDecayActive(false);

	
	if (APlayerController* PlayerController = Cast<APlayerController>(OwningCharacter->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FlyMappingContext);
			OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
	}
}

bool ACHJetpack::IsThrusting()
{
	if (bIsThrusting || bIsBoosting)
	{
		return true;

	}
	return false;
}

// Will need to make this based off the boost value instead
void ACHJetpack::ThrustUp()
{	
	if (!bIsFlying) return;

	bIsThrusting = true;

	// Step 1: Gradual boost charge increase
	BoostCharge += GetWorld()->GetDeltaSeconds();
	BoostCharge = FMath::Clamp(BoostCharge, 0, MaxBoostCharge);

	// Step 2: Map charge to usable thrust power (Alpha from 0–1)
	float Alpha = BoostCharge / MaxBoostCharge;

	UE_LOG(LogTemp, Warning, TEXT("BoostCharge %f  !"), BoostCharge);

	// Interpolate LaunchSpeed toward MaxThrust based on charge
	LaunchSpeed = FMath::FInterpTo(LaunchSpeed, FMath::Lerp(BoostStrengthMin, BoostStrengthMax, Alpha), GetWorld()->GetDeltaSeconds(), ThrustAccel);

	// Step 3: Apply upward force via AddMovementInput
	OwningCharacter->AddMovementInput(FVector::UpVector, LaunchSpeed * GetWorld()->GetDeltaSeconds());
	FVector& Velocity = OwningCharacter->GetCharacterMovement()->Velocity;
	

	Velocity.Z += LaunchSpeed * GetWorld()->GetDeltaSeconds();
	
	Velocity.Z = FMath::Clamp(Velocity.Z, abs(Velocity.Z), BoostStrengthMax);

}

void ACHJetpack::ThrustRelease()
{
	// If we aren't boosting, turn of thrust, boost and reset, get ready for Thrust to Hover Bounce
	if (!bIsBoosting)
	{

		//  Gradual Slow Down on BoostCharge
		while (BoostCharge > 1.0f)
		{
			BoostCharge -= 0.005;
			OwningCharacter->LaunchCharacter(FVector(OwningCharacter->GetCharacterMovement()->Velocity.X, OwningCharacter->GetCharacterMovement()->Velocity.Y ,  LaunchSpeed * BoostCharge * 2), true, false);
		}

		bIsStabilizing = true;
		HoverTargetZ = OwningCharacter->GetActorLocation().Z; // or however high you want to hover
		StabilizeVelocity = 0.0f; // reset

		// Will gradually slow down instead?
		BoostCharge = DefaultBoost;

		AttributeComp->IncreaseAttributeDecayValue("Fuel", -1.0f);

		LaunchSpeed = 10.0f;
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

		// Integrate velocity
		StabilizeVelocity += Accel * GetWorld()->GetDeltaSeconds();

		StabilizeVelocity = FMath::Clamp(StabilizeVelocity, -1000.0f, 1500.0f);
		Location.Z += StabilizeVelocity * GetWorld()->GetDeltaSeconds();

		OwningCharacter->SetActorLocation(Location);



		// End stabilization once close enough and velocity is low
		// Boosting Up

		if (FMath::Abs(Displacement) <= 5.0f && StabilizeVelocity <= 20.0f)
		{
 			bIsStabilizing = false;
			HoverTime = 0.0f;
			return;
		}
		if (Displacement > 100.0f && StabilizeVelocity > 1000.0f)
		{
			bIsStabilizing = false;
			HoverTime = 0.0f;
		}
		// Boosting Down

		return; // Skip hover bobbing until stabilized
	}

	if (bIsFlying && !bIsThrusting && !bIsBoosting)
	{
		OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		Hover();
	}
}

void ACHJetpack::ThrustDown()
{	
	if (!bIsFlying)	return;

	bIsThrusting = false;
	bIsBoosting = false;
	OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);



	bIsStabilizing = true;
	HoverTargetZ = OwningCharacter->GetActorLocation().Z + 50;
	StabilizeVelocity = 1.0f; // reset
	HoverTime = 0.0f;
	BoostCharge = 0.0f;
}

void ACHJetpack::ThrustBoost()
{
	bIsBoosting = true;

	FVector FinalBoost = FVector::ZeroVector;

	// Vertical thrust (optional if also using ThrustUp separately)
	if (bIsThrusting)
	{
		LaunchSpeed = FMath::FInterpTo(LaunchSpeed, MaxThrust, GetWorld()->GetDeltaSeconds(), ThrustAccel);
		FinalBoost.Z += LaunchSpeed;
	}

	// Forward boost buildup
	if (bIsBoosting)
	{
		BoostCharge += GetWorld()->GetDeltaSeconds();
		BoostCharge = FMath::Clamp(BoostCharge, 0.0f, MaxBoostCharge);

		float Alpha = BoostCharge / MaxBoostCharge;
		float ForwardPower = FMath::Lerp(BoostStrengthMin, BoostStrengthMax, Alpha);

		FVector BoostDirection = OwningCharacter->GetFirstPersonCameraComponent()->GetForwardVector().GetSafeNormal();

		FinalBoost += BoostDirection * ForwardPower;
	}

	// Optional: clamp to max boost size
	FinalBoost = FinalBoost.GetClampedToMaxSize(BoostStrengthMax); // Or MaxBoostVelocity, if you separate them

	FVector& Velocity = OwningCharacter->GetCharacterMovement()->Velocity;
	Velocity += FinalBoost * GetWorld()->GetDeltaSeconds();

	// Clamp to boost cap
	Velocity = Velocity.GetClampedToMaxSize(BoostStrengthMax);

}

void ACHJetpack::BoostRelease()
{
	bIsBoosting = false;

	HoverTime = 0.0f;



	if (!bIsThrusting)
	{
		//OwningCharacter->LaunchCharacter(BoostCharge * OwningCharacter->GetFirstPersonCameraComponent()->GetForwardVector().GetSafeNormal(), false, false);

		LaunchSpeed = 10.0f;
		bIsStabilizing = true;
		HoverTargetZ = OwningCharacter->GetActorLocation().Z; // Release height
		StabilizeVelocity = 0.0f;	 // reset

		// Should have gradual slow down instead?
		BoostCharge = DefaultBoost;
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
}

float ACHJetpack::GetCurrentFuel()
{
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

