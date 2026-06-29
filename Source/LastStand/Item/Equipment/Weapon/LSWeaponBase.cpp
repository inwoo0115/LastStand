// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "DataTable/LSDataSubsystem.h"
#include "DataTable/LSItemData.h"

void ALSWeaponBase::LaunchWeapon()
{
}

void ALSWeaponBase::ReleaseWeapon()
{
}

void ALSWeaponBase::Equipped()
{
}

void ALSWeaponBase::UnEquipped()
{
}

void ALSWeaponBase::ReloadWeapon()
{
}

void ALSWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	InitEquipment();
}

void ALSWeaponBase::InitEquipment()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	ULSDataSubsystem* Sub = GetOwner()->GetGameInstance()->GetSubsystem<ULSDataSubsystem>();
	if (!Sub)
	{
		return;
	}

	const FItemData* ID = Sub->FindItem(ItemName);
	if (!ID)
	{
		return;
	}

	// 정보 저장
	ItemData = *ID;
}
