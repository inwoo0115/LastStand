// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSEquipmentComponent.h"
#include "LSEquipmentComponent.h"
#include "Net/UnrealNetwork.h" 

// Sets default values for this component's properties
ULSEquipmentComponent::ULSEquipmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void ULSEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void ULSEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

