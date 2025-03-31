// Fill out your copyright notice in the Description page of Project Settings.


#include "CHJetpack.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "CherryHillCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
				// Thrust
				EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::Triggered, this, &ACHJetpack::ThrustUp);
				EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::Completed, this, &ACHJetpack::ThrustBounce);

				EnhancedInputComponent->BindAction(ThrustDownAction, ETriggerEvent::Triggered, this, &ACHJetpack::ThrustDown);
				EnhancedInputComponent->BindAction(ThrustDownAction, ETriggerEvent::Completed, this, &ACHJetpack::ThrustBounce);

				EnhancedInputComponent->BindAction(ThrustBoostAction, ETriggerEvent::Triggered, this, &ACHJetpack::ThrustBoost);
				EnhancedInputComponent->BindAction(ThrustBoostAction, ETriggerEvent::Completed, this, &ACHJetpack::ThrustBounce);


				EnhancedInputComponent->BindAction(ThrustUpAction, ETriggerEvent::None, this, &ACHJetpack::NoThrust);
				bIsFlying = true;
			}
		}
	}
	
}

void ACHJetpack::ThrustUp()
{	
	if (!bIsFlying)
	{
		return;
	}
	bIsThrusting = true;
	
	LaunchSpeed = FMath::FInterpTo(LaunchSpeed, MaxThrust, GetWorld()->GetDeltaSeconds(), ThrustAccel);
	OwningCharacter->LaunchCharacter(FVector(0.0f, 0.0f, LaunchSpeed), false, true);
	//UE_LOG(LogTemp, Warning, TEXT("thrust up = %f"), LaunchSpeed);
}

void ACHJetpack::ThrustBounce()
{
	// Zero out launch speed for next time
	LaunchSpeed = 10.0f;

 	bIsThrusting = false;
	bIsStabilizing = true;
	HoverTargetZ = OwningCharacter->GetActorLocation().Z; // or however high you want to hover
	StabilizeVelocity = 0.0f; // reset
	HoverTime = 0.0f;

}


// Hover Mode
void ACHJetpack::NoThrust()
{
	if (bIsStabilizing)
	{
		FVector Location = OwningCharacter->GetActorLocation();
		float CurrentZ = Location.Z;

		// Spring force toward target
		float Displacement = HoverTargetZ - CurrentZ;
		//float SpringAccel = (Displacement * SpringStrength) - (StabilizeVelocity * SpringDamping);

		// Apply spring-damper force: F = kx - cv
		float SpringForce = Displacement * SpringStrength;
		float DampingForce = -StabilizeVelocity * SpringDamping;
		float Accel = SpringForce + DampingForce;

		// Integrate velocity
		StabilizeVelocity += Accel * GetWorld()->GetDeltaSeconds();

		// Apply to position
		StabilizeVelocity = FMath::Clamp(StabilizeVelocity, -1000.0f, 1000.0f);

		Location.Z += StabilizeVelocity * GetWorld()->GetDeltaSeconds();

		OwningCharacter->SetActorLocation(Location);

		DrawDebugLine(GetWorld(), FVector(Location.X, Location.Y, HoverTargetZ - 10), FVector(Location.X, Location.Y, HoverTargetZ + 10), FColor::Green, false, -1, 0, 2.0f);


		// End stabilization once close enough and velocity is low
		if (FMath::Abs(Displacement) <= 5.0f && StabilizeVelocity <= 20.0f)
		{
 			bIsStabilizing = false;
			HoverTime = 0.0f; // start bobbing clean
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("Displacement = %f"), Displacement);
		UE_LOG(LogTemp, Warning, TEXT("StabilizeVelocity = %f"), StabilizeVelocity);

		return; // Skip hover bobbing until stabilized
	}


	float HoverAmplitude = 20.0f; // How strong the bob is
	float HoverFrequency = 5.0f;

	if (bIsFlying && !bIsThrusting)
	{
  		HoverTime += GetWorld()->GetDeltaSeconds();

		// Calculate bobbing offset
		float BobOffset = FMath::Sin(HoverTime * HoverFrequency) * HoverAmplitude;

		// Apply bobbing to velocity
		FVector Velocity = OwningCharacter->GetVelocity();
		Velocity.Z = BobOffset;
		OwningCharacter->GetCharacterMovement()->Velocity = Velocity;
		//UE_LOG(LogTemp, Warning, TEXT("velocity = %f"), BobOffset);

		UE_LOG(LogTemp, Warning, TEXT("bobor"));
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
	HoverTargetZ = OwningCharacter->GetActorLocation().Z; // or however high you want to hover
	StabilizeVelocity = 0.0f; // reset
	HoverTime = 0.0f;

}

void ACHJetpack::ThrustBoost()
{
	float LaunchSpeed2 = FMath::FInterpTo(ThrustAccel, MaxThrust, GetWorld()->GetDeltaSeconds(), 0.8f);
	OwningCharacter->LaunchCharacter(FVector(0.0f, 0.0f, 0.0f), false, false);
}

