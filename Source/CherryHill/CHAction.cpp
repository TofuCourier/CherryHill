// Fill out your copyright notice in the Description page of Project Settings.


#include "CHAction.h"
#include "CHActionComponent.h"

//void UCHAction::Initalize(UCHActionComponent* NewActionComp)
//{
//	ActionComp = NewActionComp;
//}

void UCHAction::StartAction_Implementation(AActor* Instigator)
{
	 UE_LOG(LogTemp, Log, TEXT("Running %s"), *GetNameSafe(this));
	UCHActionComponent* Comp = GetOwningComponent();
	Comp->ActiveGameplayTags.AppendTags(GrantsTags);

	bIsRunning = true;

	//GetOwningComponent()->OnActionStarted.Broadcast(GetOwningComponent(), this);
}

void UCHAction::StopAction_Implementation(AActor* Instigator)
{
	UCHActionComponent* Comp = GetOwningComponent();
	Comp->ActiveGameplayTags.AppendTags(GrantsTags);

	bIsRunning = false;

	//GetOwningComponent()->OnActionStopped.Broadcast(GetOwningComponent(), this);
}

bool UCHAction::CanStart_Implementation(AActor* Instigator)
{
	if (IsRunning())
	{
		return false;
	}

	UCHActionComponent* Comp = GetOwningComponent();

	if (Comp->ActiveGameplayTags.HasAny(BlockedTags))
	{
		return false;
	}

	return true;
}

void UCHAction::Initialize_Implementation(AActor* Instigator)
{

}

bool UCHAction::IsRunning() const
{
	return bIsRunning;
}

UWorld* UCHAction::GetWorld() const
{
	// Outer is set when creating action via NewObject<T>
	UActorComponent* Comp = Cast<UActorComponent>(GetOuter());
	if (Comp)
	{
		return Comp->GetWorld();
	}

	return nullptr;
}

UCHActionComponent* UCHAction::GetOwningComponent() const
{
	return Cast<UCHActionComponent>(GetOuter());
}
