// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSEquipmentComponent.h"
#include "LSEquipmentComponent.h"
#include "Net/UnrealNetwork.h" 
#include "DataTable/LSItemData.h"
#include "DataTable/LSDataSubsystem.h"
#include "LSInventoryComponent.h"
#include "Item/Equipment/LSEquipmentBase.h"
#include "Item/Equipment/Weapon/LSWeaponBase.h"


// Sets default values for this component's properties
ULSEquipmentComponent::ULSEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void ULSEquipmentComponent::EquipItemFromInventory(FName ItemName)
{
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: Call from Client Error"));
		return;
	}

	ULSDataSubsystem* Sub = GetOwner()->GetGameInstance()->GetSubsystem<ULSDataSubsystem>();
	if (!Sub)
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: Unavail Data sub system"));
		return;
	}

	const FItemData* ID = Sub->FindItem(ItemName);
	if (!ID)
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: Unavail Item Data"));
		return;
	}

	if (ID->EquipmentType == EEquipmentType::None)
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: It is not Equipment"));
		return;
	}

	ULSInventoryComponent* IC = GetOwner()->GetComponentByClass<ULSInventoryComponent>();
	if (!IC)
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: No Inventory"));
		return;
	}

	// Spawn Param 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	TSubclassOf<AActor> ItemClass = ID->ItemClass.LoadSynchronous();

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ItemClass, FVector(), FRotator(), SpawnParams);

	if (!SpawnedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnerComponent] 스폰 실패: %s"), *ItemClass->GetName());
	}

	// 이미 자리에 있을 경우 해제
	if (Equipments.Find(ID->EquipmentType))
	{
		UnEquipItemFromInventory(ID->EquipmentType);
	}

	// 장착
	Equipments.Add(ID->EquipmentType, SpawnedActor);

	// 인벤토리에서 제거
	IC->AddDeltaToItem(ID->ItemName, -1);
}

void ULSEquipmentComponent::UnEquipItemFromInventory(EEquipmentType EquipType)
{
	if (!Equipments.Find(EquipType))
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: No Equipment Type"));
		return;
	}

	ULSInventoryComponent* IC = GetOwner()->GetComponentByClass<ULSInventoryComponent>();
	if (!IC)
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: No Inventory"));
		return;
	}

	TObjectPtr<AActor> Equipment = *Equipments.Find(EquipType);

	ALSEquipmentBase* EB = Cast<ALSEquipmentBase>(Equipment);
	if (!EB)
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: Cast Fail to EquipmentBase"));
		return;
	}

	// 장비 제거
	EB->UnEquipped();
	Equipments.Remove(EquipType);

	// 인벤토리에 추가
	IC->AddItemToInventory(EB->GetItemName(), 1);
}

void ULSEquipmentComponent::FocusEquipmentByType(EEquipmentType EquipType)
{
	// 서버 실행
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (EquipType == EEquipmentType::None)
	{
		FocusEquipment = nullptr;
	}
	else if (Equipments.Contains(EquipType))
	{
		ALSWeaponBase* WB = Cast<ALSWeaponBase>(Equipments[EquipType]);
		if (WB)
		{
			FocusEquipment = WB;
		}
	}
}

void ULSEquipmentComponent::LaunchEquipment()
{
	FocusEquipment->LaunchWeapon();
}

void ULSEquipmentComponent::ReleaseEquipment()
{
	FocusEquipment->ReleaseWeapon();
}


void ULSEquipmentComponent::Reload()
{
	// TODO: Inventory 확인 후 총알 있을 시 리로드

	FocusEquipment->ReloadWeapon();
}



void ULSEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void ULSEquipmentComponent::OnRepFocusEquipment()
{
	// 애니메이션 출력
}


void ULSEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULSEquipmentComponent, FocusEquipment);
}

