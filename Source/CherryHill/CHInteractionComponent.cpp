// Fill out your copyright notice in the Description page of Project Settings.


#include "CHInteractionComponent.h"
#include "CHGameplayInterface.h"

// Sets default values for this component's properties
UCHInteractionComponent::UCHInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	// Since we use Camera info in Tick we want the most up-to-date camera position for tracing
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	bAutoActivate = false;
	TraceDistance = 500.0f;
	TraceRadius = 30.0f;

	CollisionChannel = ECC_WorldDynamic;
}


void UCHInteractionComponent::PrimaryInteract()
{

	APawn* MyPawn = Cast<APawn>(GetOwner());
	if (FocusedActor && MyPawn)
	{
		ICHGameplayInterface::Execute_Interact(FocusedActor, MyPawn);
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Interact");
}


void UCHInteractionComponent::FindInteractable()
{

	FCollisionObjectQueryParams ObjectsQueryParams;
	ObjectsQueryParams.AddObjectTypesToQuery(CollisionChannel);

	AActor* MyOwner = GetOwner();

	FHitResult Hits;

	FVector EyeLocation;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector EndLocation = EyeLocation + (EyeRotation.Vector() * TraceDistance);

	bool bBlockingHit = GetWorld()->LineTraceSingleByObjectType(Hits, EyeLocation, EndLocation, ObjectsQueryParams);


	FColor LineColor = bBlockingHit ? FColor::Green : FColor::Red;


	// Clear reference before filling
	FocusedActor = nullptr;
	AActor* HitActor = Hits.GetActor();

	if (HitActor && FocusedActor != HitActor)
	{
		if (HitActor->Implements<UCHGameplayInterface>())
		{
			FocusedActor = HitActor;
			
		}
	}
}

void UCHInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	FindInteractable();
}

