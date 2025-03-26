// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CHAttributeComponent.generated.h"


USTRUCT(BlueprintType)
struct FAttributes
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float Hunger;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float MaxHunger;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float HungerDecay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float Toilet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float MaxToilet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float ToiletDecay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float Sleep;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float MaxSleep;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float SleepDecay;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChange, AActor*, InstigatorActor, UCHAttributeComponent*, OwningComp,  FAttributes, Attributes);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHERRYHILL_API UCHAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCHAttributeComponent();

	// Allows easy access to get the attributes from a specific character
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	static UCHAttributeComponent* GetAttributes(AActor* FromActor);


protected:

	FTimerHandle TimerHandle_AttributeDecay;

	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	float DecayTimerInterval;

	void AttributeDecay();

	UFUNCTION()
	void AttributeDecayTimerElapsed();


	virtual void BeginPlay() override;

public:	

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChange OnAttributeChange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FAttributes Attributes;

	UFUNCTION(BlueprintCallable)
	void IncreaseHungerValue(float Value);
};
