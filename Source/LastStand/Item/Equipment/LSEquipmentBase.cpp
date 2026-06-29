// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Equipment/LSEquipmentBase.h"
#include "LSEquipmentBase.h"

void ALSEquipmentBase::Equipped()
{
}

void ALSEquipmentBase::UnEquipped()
{
	//장비 제거 구현
	Destroy();
}

void ALSEquipmentBase::InitEquipment()
{
}
