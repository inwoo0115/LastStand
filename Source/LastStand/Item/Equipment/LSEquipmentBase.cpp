// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Equipment/LSEquipmentBase.h"
#include "LSEquipmentBase.h"

ALSEquipmentBase::ALSEquipmentBase()
{
	PrimaryActorTick.bCanEverTick = false; // 기본적으로 틱 자체를 안 쓰게

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
