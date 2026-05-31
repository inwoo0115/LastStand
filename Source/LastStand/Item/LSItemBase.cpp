// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/LSItemBase.h"


// Sets default values
ALSItemBase::ALSItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

const FName ALSItemBase::GetItemName()
{
	return ItemName;
}

// Called when the game starts or when spawned
void ALSItemBase::BeginPlay()
{
	Super::BeginPlay();
	
}

