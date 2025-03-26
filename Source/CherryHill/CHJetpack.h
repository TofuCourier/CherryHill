// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CHJetpack.generated.h"

UCLASS()
class CHERRYHILL_API ACHJetpack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACHJetpack();

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* FlyMappingContext;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* ThrustAction;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SwitchToFlying();

};
