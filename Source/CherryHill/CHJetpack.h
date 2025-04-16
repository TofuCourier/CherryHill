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

	// Represents our speed affected by BoostCharge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Boost")
	float BoostSpeed;

	// Boost charge acceleration speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Boost")
	float ThrustAccel = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Jetpack|Hover")
	float HoverAmplitude = 40.0f;

	UPROPERTY(EditAnywhere, Category = "Jetpack|Hover")
	float HoverFrequency = 4.0f;




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
	UInputAction* BoostAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Deactivate Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* DeactivateJetpackAction;

	UPROPERTY(BlueprintReadOnly)
	bool bIsFlying = false;;

	UPROPERTY(BlueprintReadOnly)
	bool bIsThrusting = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsBoosting = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsStabilizing = false;

	bool bInThrustFall = false;

	// Prevents inputs from being bound more than once
	bool bJetpackInputBound = false;

	bool bNegativeStability = false;

	// 
	float PrevStabilizeVelocity = 0.0;

	float PrevDisplacement = 0.0f;

	FVector SavedLateralVelocity = FVector::ZeroVector;

	float HoverTargetZ = 0.0f; // Set when launching

	float StabilizeVelocity = 0.0f;

	float HoverTime = 0.0;

	FTimerHandle InputCheckDelayHandle;

	UFUNCTION(BlueprintPure, Category = "Jetpack|Boost")
	float GetBoostChargeAlpha() const;

	// On Jetpack activation allows the continuous thrust when inputs are being switched during Jump Activation
	void OnActivateThrustTimer(APlayerController* Controller);

	virtual void BeginPlay() override;

	// Movement
	void ThrustInitiate();

	void ThrustUp();

	void ThrustUpComplete();

	void ThrustToHover();

	void ThrustDownInitiate();

	void ThrustDown();

	void ThrustDownComplete();

	void BoostInitiate();

	void Boost();

	void BoostComplete();

	void Hover();

	void Move(const FInputActionValue& Value);


	// Boost
	void BoostChargeUp(float DeltaTime);

	void BoostChargeDown(float DeltaTime);

	// Attributes
	float GetCurrentFuel();

	bool HasFuel();


public:	

	UFUNCTION(BlueprintCallable, Category = "JetPack")
	void AttachJetpack(ACharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "JetPack")
	void OnJetpackActivated(AActor* IntigatorActor, ACHJetpack* Jetpack, bool bActivated);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "JetPack")
	void OnJetpackDeactivated();

	UFUNCTION(BlueprintCallable, Category = "JetPack")
	bool IsThrusting();

	UFUNCTION(BlueprintCallable, Category = "JetPack")
	bool IsStabilizing() { return bIsStabilizing; }

	UFUNCTION(BlueprintCallable, Category = "JetPack")
	bool IsThrustingDown() { return bInThrustFall; }

	UFUNCTION(BlueprintCallable, Category = "JetPack")
	bool IsFlying() { return bIsFlying; }

	virtual void Tick(float DeltaTime) override;

};
