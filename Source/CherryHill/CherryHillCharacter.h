// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CherryHillCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UCHInteractionComponent;
class UCHActionComponent;
class ACHJetpack;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJetpackActivate, AActor*, InstigatorActor, bool, bIsJetpackThrusting); // NEED TO ADD ATTRIBUTES FOR FUEL ETC.

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ACherryHillCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	UCHInteractionComponent* InteractionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UCHActionComponent* ActionComp;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpOrFlyAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;


	
public:
	ACherryHillCharacter();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable)
	FOnJetpackActivate OnJetpackActivate;

	UPROPERTY(EditDefaultsOnly, Category = "Jetpack")
	TSubclassOf<ACHJetpack> JetpackClass;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for Interact input */
	void Interact(const FInputActionValue& Value);

	void StartJumpOrFly();

	void StopJumpOrFly();

	void JetPackThrust();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flying)
	float ThrustAccel = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flying)
	float MaxThrust = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flying)
	float JetpackRampUpSpeed = 500.0f; // how fast thrust builds up

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flying)
	float DefaultThrust = 500.0f; // how fast thrust builds up

	UPROPERTY(BlueprintReadWrite)
	bool bJetpackActive = false;
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsFlying = false;

	UPROPERTY(BlueprintReadWrite)
	bool bHasJumped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentThrust = 0.f;


protected:
	// APawn interface
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

//float HoverTime = 0.0;
//
//if (bIsStabilizing)
//{
//	FVector Location = OwningCharacter->GetActorLocation();
//	float CurrentZ = Location.Z;
//
//	// Spring force toward target
//	float Displacement = HoverTargetZ - CurrentZ;
//
//	// Apply spring-damper force: F = kx - cv
//	float SpringAccel = (Displacement * SpringStrength) - (StabilizeVelocity * SpringDamping);
//
//	// Integrate velocity
//	StabilizeVelocity += SpringAccel * GetWorld()->GetDeltaSeconds();
//
//	StabilizeVelocity = FMath::Clamp(StabilizeVelocity, -4000.0f, 4000.0f);
//
//	// Apply to position
//	Location.Z += StabilizeVelocity * GetWorld()->GetDeltaSeconds();
//	OwningCharacter->SetActorLocation(Location);
//
//	// End stabilization once close enough and velocity is low
//	if (FMath::Abs(Displacement) < 1.0f && FMath::Abs(StabilizeVelocity) < 5.0f)
//	{
//		bIsStabilizing = false;
//		HoverTime = 0.0f; // start bobbing clean
//		StabilizeVelocity = 0.0f;
//		return; // Skip hover bobbing until stabilized
//	}
//
//	UE_LOG(LogTemp, Warning, TEXT("Displacement = %f"), Displacement);
//	UE_LOG(LogTemp, Warning, TEXT("StabilizeVelocity = %f"), OwningCharacter->GetCharacterMovement()->Velocity.Z);
//
//}
//
//float HoverAmplitude = 10.0f; // How strong the bob is
//float HoverFrequency = 2.0f;
//
//if (bIsFlying && !bIsThrusting)
//{
//	HoverTime += GetWorld()->GetDeltaSeconds();
//
//	// Calculate bobbing offset
//	float BobOffset = FMath::Sin(HoverTime * HoverFrequency) * HoverAmplitude;
//
//	// Apply bobbing to velocity
//	FVector Velocity = OwningCharacter->GetVelocity();
//	Velocity.Z = BobOffset;
//	OwningCharacter->GetCharacterMovement()->Velocity = Velocity;
//	//UE_LOG(LogTemp, Warning, TEXT("velocity = %f"), BobOffset);
//}