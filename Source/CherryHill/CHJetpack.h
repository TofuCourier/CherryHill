// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CHJetpack.generated.h"

class UInputAction;
class UInputMappingContext;
class ACherryHillCharacter;
class UCHAttributeComponent;
class UEnhancedInputComponent;
struct FInputActionValue;

UCLASS()
class CHERRYHILL_API ACHJetpack : public AActor
{
	GENERATED_BODY()
	
public:	

	ACHJetpack();

	UPROPERTY(EditAnywhere, Category = "Jetpack|Dampen")
	float SpringStrength = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Jetpack|Dampen")
	float SpringDamping = 2.0f;

	/* Current charge amount Thrusting/Boosting  @TODO Should this stay editable?*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jetpack|Boost")
	float BoostCharge = 0.5f;

	/* Default Boost amount when flying */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Boost")
	float BoostChargeDefault = 1.0f; 

	/* Maximum boost charge amount when Thrusting/Boosting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jetpack|Boost")
	float BoostChargeMax = 3.0f; // seconds to max

	/* Max Speed when Boosting cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jetpack|Boost")
	float BoostSpeedMax = 1000.0f;

	/* Min Speed when Boosting cm/s */
	UPROPERTY(EditAnywhere, Category = "Jetpack|Boost")
	float BoostSpeedMin = 0.0f;

	//UPROPERTY(EditAnywhere, Category = "Jetpack|Speed")
	float SpeedDefault = 400.0f;


	UPROPERTY(EditAnywhere, Category = "Jetpack|Hover")
	float HoverAmplitude = 40.0f;

	UPROPERTY(EditAnywhere, Category = "Jetpack|Hover")
	float HoverFrequency = 4.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Thrust")
	float ThrustAccel = 0.1f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Thrust")
	float BoostSpeed; // how fast thrust builds up



protected:

	UPROPERTY()
	ACherryHillCharacter* OwningCharacter = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AttributeComp", meta = (AllowPrivateAccess = "true"))
	UCHAttributeComponent* AttributeComp;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputMappingContext* FlyMappingContext;

	/** Thrust Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* ThrustUpAction;

	/** Thrust Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* ThrustDownAction;

	/** Thrust Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* ThrustBoostAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Deactivate Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* DeactivateJetpackAction;



	UPROPERTY(BlueprintReadOnly)
	bool bJetpackActive = false;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsFlying = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsThrusting = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsBoosting = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsStabilizing = false;

	// Within stable range - may not be needed
	bool bWithinStableRange = false;

	// Prevents inputs from being bound more than once
	bool bJetpackInputBound = false;

	// 
	float PrevD = 0.0;
	float PrevV = 0.0;

	float HoverTargetZ = 0.0f; // Set when launching

	float StabilizeVelocity = 0.0f;

	float HoverTime = 0.0;

	FTimerHandle InputCheckDelayHandle;

	virtual void BeginPlay() override;

	// On Jetpack activation allows the continuous thrust when inputs are being switched during Jump Activation
	void OnActivateThrustTimer(APlayerController* Controller);


	void ThrustInitiate();


	void ThrustRelease();

	void ThrustToHover();

	void ThrustDown();

	void ThrustBoost();

	void BoostRelease();

	void Hover();

	float GetCurrentFuel();

	bool HasFuel();

	void BoostChargeUp(float DeltaTime);

	void BoostChargeDown(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Jetpack|Boost")
	float GetBoostChargeAlpha() const;

	void Move(const FInputActionValue& Value);

public:	

	UFUNCTION(BlueprintCallable, Category = "JetPack")
	void AttachJetpack(ACharacter* TargetCharacter);

	UFUNCTION()
	void OnJetpackActivate(AActor* IntigatorActor, ACHJetpack* Jetpack, bool bIsJetpackThrusting);

	void JetpackDeactivate();

	bool IsThrusting();

	bool IsStabilizing() { return bIsStabilizing; }

	virtual void Tick(float DeltaTime) override;

	void ThrustUp();
};
