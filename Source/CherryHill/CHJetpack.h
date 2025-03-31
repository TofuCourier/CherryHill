// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CHJetpack.generated.h"

class ACherryHillCharacter;

UCLASS()
class CHERRYHILL_API ACHJetpack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACHJetpack();

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	ACherryHillCharacter* OwningCharacter = nullptr;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* FlyMappingContext;

	/** Thrust Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* ThrustUpAction;

	/** Thrust Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* ThrustDownAction;

	/** Thrust Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* ThrustBoostAction;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flying)
	float ThrustAccel = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flying)
	float MaxThrust = 20.0f;

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

	bool bBoundAction = false;

	float CurrentThrust = 0.f;

	bool bWasThrustingLastFrame = false;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "JetPack")
	void AttachJetpack(ACharacter* TargetCharacter);

	UFUNCTION()
	void OnJetpackActivate(AActor* IntigatorActor, bool bIsJetpackThrusting);

	void ThrustUp();

	void ThrustBounce();

	void NoThrust();

	void ThrustDown();

	void ThrustBoost();





	bool bIsStabilizing = false;

float HoverTargetZ = 0.0f; // Set when launching
float StabilizeVelocity = 0.0f;

UPROPERTY(EditAnywhere)
float SpringStrength = 10.0f;

UPROPERTY(EditAnywhere)
float SpringDamping = 2.0f;

float HoverTime = 0.0;



};
