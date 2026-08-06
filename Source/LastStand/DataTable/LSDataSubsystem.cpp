// Fill out your copyright notice in the Description page of Project Settings.


#include "DataTable/LSDataSubsystem.h"
#include "Settings/LSGameDataSettings.h"
#include "LSItemData.h"
#include "LSWeaponData.h"
#include "LSDataSubsystem.h"

void ULSDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 세션 시작하면 로드 하기
	const ULSGameDataSettings* Settings = GetDefault<ULSGameDataSettings>();
	ItemTable = Settings->ItemTable.LoadSynchronous();
	WeaponTable = Settings->WeaponTable.LoadSynchronous();
	EnemyTable = Settings->EnemyTable.LoadSynchronous();
}

const FItemData* ULSDataSubsystem::FindItem(FName ItemID) const
{
	if (!ItemTable)
	{
		UE_LOG(LogTemp, Log, TEXT("Cannot Found ItemTable"));

		return nullptr;
	}

	return ItemTable->FindRow<FItemData>(ItemID, TEXT("UDataSubsystem::FindItem"));
}

const FWeaponData* ULSDataSubsystem::FindWeapon(FName ItemID) const
{
	if (!WeaponTable)
	{
		UE_LOG(LogTemp, Log, TEXT("Cannot Found WeaponTable"));

		return nullptr;
	}

	return WeaponTable->FindRow<FWeaponData>(ItemID, TEXT("UDataSubsystem::FindWeapon"));
}

const FEnemyData* ULSDataSubsystem::FindEnemy(FName EnemyID) const
{
	if (!EnemyTable)
	{
		UE_LOG(LogTemp, Log, TEXT("Cannot Found EnemyTable"));

		return nullptr;
	}

	return EnemyTable->FindRow<FEnemyData>(EnemyID, TEXT("UDataSubsystem::FindEnemy"));
}
