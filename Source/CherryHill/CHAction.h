// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "CHAction.generated.h"


class UCHActionComponent;
class UWorld;

UCLASS(Blueprintable)
class CHERRYHILL_API UCHAction : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "Action")
	FName ActionName;
	
protected:

	UPROPERTY()
	UCHActionComponent* ActionComp;

	// Tags added to owning container when activated, removed when action ends
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer GrantsTags;

	// Tags that owning container cant use if another action is running
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSoftObjectPtr<UTexture2D> Icon;

	UFUNCTION(BlueprintCallable, Category = "Action")
	UCHActionComponent* GetOwningComponent() const;

	UPROPERTY()
	float TimeStarted;

	bool bIsRunning;

public:

	// We don't want to have to call start action manually whenever an action is added, so AutoStart
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	bool bAutoStart;
	
	//void Initalize(UCHActionComponent* NewActionComp);

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	void StartAction(AActor* Instigator);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
	void StopAction(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	bool CanStart(AActor* Instigator);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
	void Initialize(AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool IsRunning() const;


	// We want access to the world in Blueprints
	virtual UWorld* GetWorld() const override;

};
