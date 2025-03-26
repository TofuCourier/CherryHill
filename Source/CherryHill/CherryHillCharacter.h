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

	UPROPERTY(EditDefaultsOnly, Category = "Jetpack")
	TSubclassOf<ACHJetpack> JetpackClass;

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
	bool bIsThrusting = false;

	bool bBoundAction = false;

	float CurrentThrust = 0.f;

	bool bWasThrustingLastFrame = false;

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

