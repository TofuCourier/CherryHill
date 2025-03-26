// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CHInteractionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHERRYHILL_API UCHInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCHInteractionComponent();

	void PrimaryInteract();

protected:

	UPROPERTY(Transient)
	TObjectPtr<AActor> FocusedActor;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float TraceDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float TraceRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_WorldDynamic;

	//UPROPERTY(EditDefaultsOnly, Category = "UI")
	//TSubclassOf<UCHBaseUserWidget> DefaultWidgetClass = nullptr;

	//UPROPERTY(VisibleAnywhere, Category = "UI")
	//UCHBaseUserWidget* DefaultWidgetInstance = nullptr;

	void FindInteractable();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
