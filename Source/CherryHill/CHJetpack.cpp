// Fill out your copyright notice in the Description page of Project Settings.


#include "CHJetpack.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "CherryHillCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
ACHJetpack::ACHJetpack()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACHJetpack::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACHJetpack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACHJetpack::AttachJetpack(ACharacter* TargetCharacter)
{	
	OwningCharacter = Cast<ACherryHillCharacter>(TargetCharacter);

	if (OwningCharacter)
	{
		this->AttachToActor(TargetCharacter, FAttachmentTransformRules::KeepRelativeTransform);
		OwningCharacter->OnJetpackActivate.AddDynamic(this, &ACHJetpack::OnJetpackActivate);
		if (GetClass()->IsChildOf(ACHJetpack::StaticClass()))
		{
			OwningCharacter->JetpackClass = GetClass();
		}
	}
}

void ACHJetpack::OnJetpackActivate(AActor* IntigatorActor, bool bIsJetpackThrusting)
{
	if (bIsJetpackThrusting)
	{
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
				EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::Triggered, this, &ACHJetpack::ThrustUp);
				EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::Completed, this, &ACHJetpack::ThrustRelease);

				// Thrust Down
				EnhancedInputComponent->BindAction(ThrustDownAction, ETriggerEvent::Triggered, this, &ACHJetpack::ThrustDown);

				// Thrust Boost
				EnhancedInputComponent->BindAction(ThrustBoostAction, ETriggerEvent::Triggered, this, &ACHJetpack::ThrustBoost);
				EnhancedInputComponent->BindAction(ThrustBoostAction, ETriggerEvent::Completed, this, &ACHJetpack::ThrustRelease);

				// No Thrust
				EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::None, this, &ACHJetpack::ThrustToHover);

				// Deactivate
				EnhancedInputComponent->BindAction(DeactivateJetpackAction, ETriggerEvent::Completed, this, &ACHJetpack::JetpackDeactivate);


				OwningCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
				bIsFlying = true;
			}
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

	
	if (APlayerController* PlayerController = Cast<APlayerController>(OwningCharacter->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FlyMappingContext);
		}
	}
}

// Will need to make this based off the boost value instead
void ACHJetpack::ThrustUp()
{	
	if (!bIsFlying)
	{
		return;
	}
	bIsThrusting = true;
	
	LaunchSpeed = FMath::FInterpTo(LaunchSpeed, MaxThrust, GetWorld()->GetDeltaSeconds(), ThrustAccel);
	OwningCharacter->LaunchCharacter(FVector(0.0f, 0.0f, LaunchSpeed), false, true);
}

void ACHJetpack::ThrustRelease()
{
	// Zero out launch speed for next time
	LaunchSpeed = 10.0f;

 	bIsThrusting = false;
	bIsStabilizing = true;
	HoverTargetZ = OwningCharacter->GetActorLocation().Z; // or however high you want to hover
	StabilizeVelocity = 0.0f; // reset
	HoverTime = 0.0f;

	// Will gradually slow down instead?
	BoostCharge = 0.0f;
}


void ACHJetpack::ThrustToHover()
{
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

		// Boosting Down
		if (Displacement > 100.0f && StabilizeVelocity > 1000.0f)
		{
			bIsStabilizing = false;
			HoverTime = 0.0f;
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("Displacement = %f"), Displacement);
		UE_LOG(LogTemp, Warning, TEXT("StabilizeVelocity = %f"), StabilizeVelocity);

		return; // Skip hover bobbing until stabilized
	}

	if (bIsFlying && !bIsThrusting)
	{
		Hover();
	}
}

void ACHJetpack::ThrustDown()
{	
	if (!bIsFlying)
	{
		return;
	}

	bIsThrusting = false;
	bIsStabilizing = true;
	HoverTargetZ = OwningCharacter->GetActorLocation().Z + 25;
	StabilizeVelocity = 0.0f; // reset
	HoverTime = 0.0f;

	UE_LOG(LogTemp, Warning, TEXT("THRUST DOWN"));
}

void ACHJetpack::ThrustBoost()
{
	// Charge up boost
	BoostCharge += GetWorld()->GetDeltaSeconds();
	BoostCharge = FMath::Clamp(BoostCharge, 0.0f, MaxBoostCharge);

	// Map charge time to thrust power
	float ChargeAlpha = BoostCharge / MaxBoostCharge;
	float CurrentBoostPower = FMath::Lerp(BoostStrengthMin, BoostStrengthMax, ChargeAlpha);

	// Apply force in camera direction
	if (OwningCharacter->GetFirstPersonCameraComponent())
	{
		FVector BoostDirection = OwningCharacter->GetFirstPersonCameraComponent()->GetForwardVector().GetSafeNormal();
		FVector BoostVelocity = BoostDirection * CurrentBoostPower;

		// Apply velocity (additive or replace)
		OwningCharacter->LaunchCharacter(BoostVelocity, true, true);

		UE_LOG(LogTemp, Warning, TEXT("Jetpack Boosting: %.0f"), CurrentBoostPower);
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

