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

	/* Current charge amount Thrusting/Boosting  - Currently editable for debugging*/
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

	/* Represents our speed affected by BoostCharge*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Boost")
	float BoostSpeed;

	/* Boost charge acceleration speed*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Boost")
	float ThrustAccel = 0.1f;

	/* Hover Displacement */
	UPROPERTY(EditAnywhere, Category = "Jetpack|Hover")
	float HoverAmplitude = 40.0f;

	/* Hover Speed */
	UPROPERTY(EditAnywhere, Category = "Jetpack|Hover")
	float HoverFrequency = 4.0f;

	/* Default fly speed */
	UPROPERTY(BlueprintReadOnly, Category = "Jetpack|Speed")
	float SpeedDefault = 400.0f;

protected:

	UPROPERTY()
	TObjectPtr<ACherryHillCharacter> OwningCharacter = nullptr;


	/******  Components  *****/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AttributeComp", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCHAttributeComponent> AttributeComp;


	/******  MappingContext  *****/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> FlyMappingContext;


	/******  Input Actions  ******/
	/* Ascending Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AscendAction;

	/* Descending Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DescendAction;

	/* Forwards Boost Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> BoostAction;

	/* Movement Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/* Deactivate Jetpack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DeactivateJetpackAction;


	/******  Attributes  ******/

	UFUNCTION(BlueprintPure, Category = "Jetpack|Attributes")
	float GetCurrentFuel();

	UFUNCTION(BlueprintPure, Category = "Jetpack|Attributes")
	bool HasFuel();

	UFUNCTION(BlueprintPure, Category = "Jetpack|Boost")
	float GetBoostChargeAlpha() const;


	virtual void BeginPlay() override;

public:	

	UFUNCTION(BlueprintCallable, Category = "JetPack")
	void AttachJetpack(ACharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "JetPack")
	void OnJetpackActivated(AActor* IntigatorActor, ACHJetpack* Jetpack, bool bActivated);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "JetPack")
	void OnJetpackDeactivated();

	UFUNCTION(BlueprintPure, Category = "JetPack")
	bool IsLaunching() { return bIsBoosting || bIsAscending; };

	UFUNCTION(BlueprintPure, Category = "JetPack")
	bool IsStabilizing() { return bIsStabilizing; }

	UFUNCTION(BlueprintPure, Category = "JetPack")
	bool IsDescending() { return bIsDescending; }

	UFUNCTION(BlueprintPure, Category = "JetPack")
	bool IsFlying() { return bIsFlying; }

	virtual void Tick(float DeltaTime) override;

private:

	/****** Movement ******/
	void InitiateAscend();

	void Ascend();

	void AscendComplete();

	void TransitionToHover();

	void InitiateDescend();

	void Descend();

	void DescendComplete();

	void BoostInitiate();

	void Boost();

	void BoostComplete();

	void Hover();

	void Move(const FInputActionValue& Value);

	/****** Boost ******/
	void BoostChargeUp(float DeltaTime);

	void BoostChargeDown(float DeltaTime);

	// Allows the Jetpack to continue ascending when switching inputs 
	void OnActivateHeldTimer(APlayerController* Controller);

private:

	// The previous frames velocity
	float PrevStabilizeVelocity = 0.0;

	// When character is done thrusting/falling, set target Z to hover at
	float HoverTargetZ = 0.0f; 

	// How long actor has been hovering, Related to bobbing.
	float HoverTime = 0.0;

	// Jetpack is on and character is in the air
	bool bIsFlying = false;;

	// Actor is thrusting upwards
	bool bIsAscending = false;

	// Actor is boosting forwards
	bool bIsBoosting = false;

	// Actor is stabilizing to a hover after thrusting
	bool bIsStabilizing = false;

	// Actor is descending quickly
	bool bIsDescending = false;

	// Prevents inputs from being bound more than once
	bool bJetpackInputBound = false;

	// Smooth lateral velocity transitions when falling and thrusting
	FVector SavedLateralVelocity = FVector::ZeroVector;

	// Timer for 'OnActivateThrustTimer'
	FTimerHandle InputCheckDelayHandle;
};
