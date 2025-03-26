// Fill out your copyright notice in the Description page of Project Settings.


#include "CHJetpack.h"

// Sets default values
ACHJetpack::ACHJetpack()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACHJetpack::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACHJetpack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACHJetpack::SwitchToFlying()
{

}

