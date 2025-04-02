// Fill out your copyright notice in the Description page of Project Settings.


#include "CHAttributeComponent.h"
#include "Kismet/GameplayStatics.h"

UCHAttributeComponent::UCHAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	DecayTimerInterval = 5.0f;
}

TMap<FName, FAttribute> UCHAttributeComponent::GetAttributes(AActor* FromActor)
{
	if (FromActor)
	{
		UCHAttributeComponent* Comp = FromActor->FindComponentByClass<UCHAttributeComponent>();
		return Comp->Attributes;
	}

	return TMap<FName, FAttribute>();
}

void UCHAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	GetOwner()->GetWorldTimerManager().SetTimer(TimerHandle_AttributeDecay, this, &UCHAttributeComponent::AttributeDecayTimerElapsed, DecayTimerInterval, true);
}


void UCHAttributeComponent::AttributeDecayTimerElapsed()
{
	AttributeDecay();

	OnAttributeChange.Broadcast(GetOwner(), this);
}

void UCHAttributeComponent::AttributeDecay()
{
	for (TPair<FName, FAttribute>& AttributePair : Attributes)
	{
		FAttribute& Attribute = AttributePair.Value;

		Attribute.CurrentValue += Attribute.DecayValue;

		// Maybe add a clamp? to the value when reaching 0?
	}

}


void UCHAttributeComponent::AddAttribute(FName Name, float Value = 100.0f, float MaxValue = 100.0f, float DecayValue = -1.0f)
{
	if (Attributes.Contains(Name))
	{
		return;
	}

	FAttribute NewAttributeStats;
	NewAttributeStats.CurrentValue = Value;
	NewAttributeStats.MaxValue = MaxValue;
	NewAttributeStats.DecayValue = DecayValue;

	Attributes.Add(Name, NewAttributeStats);

}

void UCHAttributeComponent::RemoveAttribute(FName Name)
{
	if (!Attributes.Contains(Name))
	{
		return;
	}

	Attributes.Remove(Name);
}

// TO BE FIXED ?
void UCHAttributeComponent::IncreaseAttributeValue(FName Name, float Value)
{
	if (Attributes.Contains(Name))
	{
		Attributes[Name].CurrentValue = FMath::Clamp(Attributes[Name].CurrentValue + Value, 0.0f, Attributes[Name].MaxValue);
	}
}
