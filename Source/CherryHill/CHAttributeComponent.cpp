// Fill out your copyright notice in the Description page of Project Settings.


#include "CHAttributeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Templates/Tuple.h"

UCHAttributeComponent::UCHAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	DecayTimerInterval = 5.0f;
}

TMap<FName, FAttribute> UCHAttributeComponent::GetAttributes(AActor* FromActor, UCHAttributeComponent*& OutComponent)
{
	if (FromActor->FindComponentByClass<UCHAttributeComponent>())
	{
		UCHAttributeComponent* Comp = FromActor->FindComponentByClass<UCHAttributeComponent>();
		OutComponent = Comp;
		return Comp->Attributes;
	}

	// Otherwise return empty
	return TMap<FName, FAttribute>();
}

void UCHAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	GetOwner()->GetWorldTimerManager().SetTimer(TimerHandle_AttributeDecay, this, &UCHAttributeComponent::AttributeDecayTimerElapsed, DecayTimerInterval, true);

}


void UCHAttributeComponent::AttributeDecayTimerElapsed()
{
	AttributeDecay(bDecayActivate);

	OnAttributeChange.Broadcast(GetOwner(), this);
}

void UCHAttributeComponent::AttributeDecay(bool bDecay)
{
	if (bDecay)
	{
		for (TPair<FName, FAttribute>& AttributePair : Attributes)
		{
			FAttribute& Attribute = AttributePair.Value;

			Attribute.CurrentValue += Attribute.DecayValue;

			// Don't go below zero
			Attribute.CurrentValue = FMath::Clamp(Attribute.CurrentValue, 0, Attribute.MaxValue);
		}
	}
}


void UCHAttributeComponent::AddAttribute(FName Name, float Value = 100.0f, float MaxValue = 100.0f, float DecayValue = -1.0f)
{
	if (Attributes.Contains(Name))
	{
		return;
	}

	// Set the new attributes variables
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

void UCHAttributeComponent::IncreaseAttributeCurrentValue(FName Name, float Value)
{
	if (Attributes.Contains(Name))
	{
		Attributes[Name].CurrentValue = FMath::Clamp(Attributes[Name].CurrentValue + Value, 0.0f, Attributes[Name].MaxValue);

		OnAttributeChange.Broadcast(GetOwner(), this);
	}
}

void UCHAttributeComponent::IncreaseAttributeDecayValue(FName Name, float Value)
{
	if (Attributes.Contains(Name))
	{
		Attributes[Name].DecayValue = Value;
	}
}
