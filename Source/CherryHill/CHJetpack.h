// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CHJetpack.generated.h"

class UInputAction;
class UInputMappingContext;
class ACherryHillCharacter;

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


	UPROPERTY(EditAnywhere, Category = "Jetpack|Boost")
	float BoostCharge = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Jetpack|Boost")
	float MaxBoostCharge = 1.5f; // seconds to max

	UPROPERTY(EditAnywhere, Category = "Jetpack|Boost")
	float BoostStrengthMax = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Jetpack|Boost")
	float BoostStrengthMin = 300.0f;


	UPROPERTY(EditAnywhere, Category = "Jetpack|Hover")
	float HoverAmplitude = 40.0f;

	UPROPERTY(EditAnywhere, Category = "Jetpack|Hover")
	float HoverFrequency = 4.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Thrust")
	float ThrustAccel = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jetpack|Thrust")
	float MaxThrust = 20.0f;

protected:

	UPROPERTY()
	ACherryHillCharacter* OwningCharacter = nullptr;

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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flying)
	float LaunchSpeed; // how fast thrust builds up

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flying)
	float DefaultThrust = 50.0f; // flying speed??? flying speed is 600




	UPROPERTY(BlueprintReadWrite)
	bool bJetpackActive = false;
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsFlying = false;

	UPROPERTY(BlueprintReadWrite)
	bool bHasJumped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsThrusting = false;

	bool bIsStabilizing = false;

	float HoverTargetZ = 0.0f; // Set when launching

	float StabilizeVelocity = 0.0f;

	float HoverTime = 0.0;

	virtual void BeginPlay() override;

public:	


	UFUNCTION(BlueprintCallable, Category = "JetPack")
	void AttachJetpack(ACharacter* TargetCharacter);

	UFUNCTION()
	void OnJetpackActivate(AActor* IntigatorActor, bool bIsJetpackThrusting);

	void JetpackDeactivate();

	void ThrustUp();

	void ThrustRelease();

	void ThrustToHover();

	void ThrustDown();

	void ThrustBoost();

	void Hover();

	virtual void Tick(float DeltaTime) override;






};
