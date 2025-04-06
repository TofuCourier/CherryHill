// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CHJetpack.generated.h"

class UInputAction;
class UInputMappingContext;
class ACherryHillCharacter;
class UCHAttributeComponent;

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

	/* Current charge amount Thrusting/Boosting */
	UPROPERTY(EditAnywhere, Category = "Jetpack|Boost")
	float BoostCharge = 0.0f;

	/* Maximum boost charge amount when Thrusting/Boosting */
	UPROPERTY(EditAnywhere, Category = "Jetpack|Boost")
	float MaxBoostCharge = 3.0f; // seconds to max

	/* Default Boost amount when flying */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Thrust")
	float DefaultBoost = 1.0f; 

	UPROPERTY(EditAnywhere, Category = "Jetpack|Boost")
	float BoostStrengthMax = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Jetpack|Boost")
	float BoostStrengthMin = 300.0f;


	UPROPERTY(EditAnywhere, Category = "Jetpack|Hover")
	float HoverAmplitude = 40.0f;

	UPROPERTY(EditAnywhere, Category = "Jetpack|Hover")
	float HoverFrequency = 4.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Thrust")
	float ThrustAccel = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Thrust")
	float MaxThrust = 20.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Thrust")
	float LaunchSpeed; // how fast thrust builds up



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

	float HoverTargetZ = 0.0f; // Set when launching

	float StabilizeVelocity = 0.0f;

	float HoverTime = 0.0;

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

public:	

	UFUNCTION(BlueprintCallable, Category = "JetPack")
	void AttachJetpack(ACharacter* TargetCharacter);

	UFUNCTION()
	void OnJetpackActivate(AActor* IntigatorActor, ACHJetpack* Jetpack, bool bIsJetpackThrusting);

	void JetpackDeactivate();

	bool IsThrusting();

	virtual void Tick(float DeltaTime) override;

	void ThrustUp();
};
