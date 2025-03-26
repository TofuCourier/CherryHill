// Fill out your copyright notice in the Description page of Project Settings.


#include "CHAttributeComponent.h"
#include "Kismet/GameplayStatics.h"

UCHAttributeComponent::UCHAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Attributes.MaxHunger = 100.0f;
	Attributes.Hunger = Attributes.MaxHunger;
	Attributes.HungerDecay = -1.0f;

	Attributes.MaxToilet = 100.0f;
	Attributes.Toilet = Attributes.MaxToilet;
	Attributes.ToiletDecay = -2.0f;

	Attributes.MaxSleep = 100.0f;
	Attributes.Sleep = Attributes.MaxSleep;
	Attributes.SleepDecay = -0.5f;

	DecayTimerInterval = 5.0f;
}

UCHAttributeComponent* UCHAttributeComponent::GetAttributes(AActor* FromActor)
{
	if (FromActor)
	{
		return Cast<UCHAttributeComponent>(FromActor->GetComponentByClass(UCHAttributeComponent::StaticClass()));
	}
	return nullptr;
}

void UCHAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	GetOwner()->GetWorldTimerManager().SetTimer(TimerHandle_AttributeDecay, this, &UCHAttributeComponent::AttributeDecayTimerElapsed, DecayTimerInterval, true);
}


void UCHAttributeComponent::AttributeDecayTimerElapsed()
{
	AttributeDecay();

	OnAttributeChange.Broadcast(GetOwner(), this, Attributes);
}

void UCHAttributeComponent::AttributeDecay()
{
	Attributes.Hunger += Attributes.HungerDecay;
	Attributes.Toilet += Attributes.ToiletDecay;
	Attributes.Sleep += Attributes.SleepDecay;

}

void UCHAttributeComponent::IncreaseHungerValue(float Value)
{
	Attributes.Hunger = FMath::Clamp(Attributes.Hunger + Value, 0.0f, Attributes.MaxHunger);

	OnAttributeChange.Broadcast(GetOwner(), this, Attributes);
}
