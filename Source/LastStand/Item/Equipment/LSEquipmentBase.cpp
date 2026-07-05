// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Equipment/LSEquipmentBase.h"
#include "LSEquipmentBase.h"

ALSEquipmentBase::ALSEquipmentBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SetActorHiddenInGame(true);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ALSEquipmentBase::Equipped()
{
}

void ALSEquipmentBase::UnEquipped()
{
	//장비 제거 구현
	UE_LOG(LogTemp, Log, TEXT("ALSEquipmentBase::UnEquipped()"));

	Destroy();
}

void ALSEquipmentBase::ActivateEquipment()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
}

void ALSEquipmentBase::DeActivateEquipment()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

void ALSEquipmentBase::BeginPlay()
{
	Super::BeginPlay();
	
	InitEquipment();
}

void ALSEquipmentBase::InitEquipment()
{
}
