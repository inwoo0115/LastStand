// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/LSItemBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ALSItemBase::ALSItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
}

const FName ALSItemBase::GetItemName()
{
	return ItemName;
}

void ALSItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALSItemBase, ItemName);
}


// Called when the game starts or when spawned
void ALSItemBase::BeginPlay()
{
	Super::BeginPlay();
	
}

