// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CHAttributeComponent.generated.h"


USTRUCT(BlueprintType)
struct FAttribute
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float CurrentValue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float MaxValue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float DecayValue;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChange, AActor*, InstigatorActor, UCHAttributeComponent*, OwningComp);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHERRYHILL_API UCHAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCHAttributeComponent();

	// Allows easy access to get the attributes from a specific character
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	static TMap<FName, FAttribute> GetAttributes(AActor* FromActor);


protected:

	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	float DecayTimerInterval;

	FTimerHandle TimerHandle_AttributeDecay;
	void AttributeDecay();

	UFUNCTION()
	void AttributeDecayTimerElapsed();

	virtual void BeginPlay() override;

public:	

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChange OnAttributeChange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	TMap<FName, FAttribute> Attributes;

	UFUNCTION(BlueprintCallable)
	void AddAttribute(FName Name, float Value, float MaxValue, float DecayValue);
	
	UFUNCTION(BlueprintCallable)
	void RemoveAttribute(FName Name);

	UFUNCTION(BlueprintCallable)
	void IncreaseAttributeValue(FName Name, float Value);
};