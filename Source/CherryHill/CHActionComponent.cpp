// Fill out your copyright notice in the Description page of Project Settings.


#include "CHActionComponent.h"
#include "CHAction.h"

// Sets default values for this component's properties
UCHActionComponent::UCHActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UCHActionComponent::BeginPlay()
{
	Super::BeginPlay();

	for (TSubclassOf<UCHAction> ActionClass : DefaultActions)
	{
		AddAction(GetOwner(), ActionClass);
	}
	
}

void UCHActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Stop All
	TArray <UCHAction*> ActionsCopy = Actions;
	for (UCHAction* Action : ActionsCopy)
	{
		if (Action && Action->IsRunning())
		{
			Action->StopAction(GetOwner());
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UCHActionComponent::AddAction(AActor* Instigator, TSubclassOf<UCHAction> ActionClass)
{
	if (!(ActionClass))
	{
		return;
	}


	UCHAction* NewAction = NewObject <UCHAction>(GetOwner(), ActionClass);
	check(NewAction);

	if (Actions.Contains(NewAction))
	{
		return;
	}

	Actions.Add(NewAction);
	NewAction->Initialize(Instigator);

	if (NewAction->bAutoStart && NewAction->CanStart(Instigator))
	{
		NewAction->StartAction(Instigator);
	}
}

void UCHActionComponent::RemoveAction(UCHAction* Action)
{
	// Make sure action is not a Nullptr and it is not running
	if (!ensure(Action && !Action->IsRunning()))
	{
		return;
	}

	Actions.Remove(Action);
}

bool UCHActionComponent::StartActionByName(AActor* Instigator, FName ActionName)
{
	for (UCHAction* Action : Actions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			// Bookmark for Unreal insights
		//	TRACE_BOOKMARK(TEXT("StartAction::%s"), *GetNameSafe(Action));
				Action->StartAction(Instigator);
				return true;
		}
	}
	return false;
}

bool UCHActionComponent::StopActionByName(AActor* Instigator, FName ActionName)
{
	for (UCHAction* Action : Actions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			if (Action->IsRunning())
			{
				Action->StopAction(Instigator);
				return true;
			}
		}
	}
	return false;
}




// Called every frame
void UCHActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//	FString DebugMessage = GetNameSafe(GetOwner()) + "  :  " + ActiveGameplayTags.ToStringSimple();
//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, DebugMessage);

	for (UCHAction* Action : Actions)
	{
		FColor TextColor = Action->IsRunning() ? FColor::Blue : FColor::White;

		//FString ActionMsg = FString::Printf(TEXT("[%s] Action: %s : IsRunning: %s"), *GetNameSafe(GetOwner()), *GetNameSafe(Action));

		//LogOnScreen(this, ActionMsg, TextColor, 0.0f);
	}

}

