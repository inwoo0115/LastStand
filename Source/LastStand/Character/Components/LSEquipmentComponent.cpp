// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSEquipmentComponent.h"
#include "LSEquipmentComponent.h"
#include "Net/UnrealNetwork.h" 
#include "DataTable/LSItemData.h"
#include "DataTable/LSDataSubsystem.h"
#include "LSInventoryComponent.h"
#include "Item/Equipment/LSEquipmentBase.h"
#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "DataTable/LSWeaponData.h"
#include "Data/LSWeaponInfoData.h"
#include "GameFramework/Character.h"

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

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: Unavail Owner Character"));
		return;
	}

	ULSDataSubsystem* Sub = GetOwner()->GetGameInstance()->GetSubsystem<ULSDataSubsystem>();
	if (!Sub)
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: Unavail Data sub system"));
		return;
	}

	const FWeaponData* ID = Sub->FindWeapon(ItemName);
	if (!ID)
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: Unavail Item Data"));
		return;
	}

	ULSInventoryComponent* IC = GetOwner()->GetComponentByClass<ULSInventoryComponent>();
	if (!IC)
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: No Inventory"));
		return;
	}

	if (!OwnerCharacter->GetMesh()->DoesSocketExist(ID->SocketName))
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: No avail socket"));
		return;
	}

	// Spawn Param 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	TSubclassOf<AActor> WeaponClass = ID->WeaponClass.LoadSynchronous();

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(WeaponClass, OwnerCharacter->GetMesh()->GetSocketLocation(ID->SocketName), OwnerCharacter->GetMesh()->GetSocketRotation(ID->SocketName), SpawnParams);

	if (!SpawnedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnerComponent] 스폰 실패: %s"), *WeaponClass->GetName());
		return;
	}

	// 소켓에 부착
	SpawnedActor->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		ID->SocketName
	);

	// 장비 타입 체크
	const EEquipmentType EquipType = ID->WeaponType;

	// 이미 장착한 무기가 있을 경우 인벤토리로 이동
	if (Equipments.Contains(EquipType))
	{
		UnEquipItemFromInventory(EquipType);
	}

	// 인벤토리에서 제거
	IC->AddDeltaToItem(ID->ItemName, -1);

	// 배열과 map 갱신
	Equipments.Add(EquipType, SpawnedActor);
	ReplicatedEquipments.Add(SpawnedActor);

	// 장착 무기 활성화
	FocusEquipmentByType(EquipType);
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
	ReplicatedEquipments.RemoveSingle(Equipment);
	Equipments.Remove(EquipType);
	EB->UnEquipped();
	
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

	// 기존 장비 해제
	if (FocusEquipment)
	{
		FocusEquipment->DeActivateEquipment();
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
			FocusEquipment->ActivateEquipment();
		}
	}
}

void ULSEquipmentComponent::LaunchEquipment()
{
	if (!FocusEquipment)
	{
		return;
	}
	FocusEquipment->LaunchWeapon();
}

void ULSEquipmentComponent::ReleaseEquipment()
{
	if (!FocusEquipment)
	{
		return;
	}
	FocusEquipment->ReleaseWeapon();
}


void ULSEquipmentComponent::Reload()
{
	// TODO: Inventory 확인 후 총알 있을 시 리로드
	if (!FocusEquipment)
	{
		return;
	}
	FocusEquipment->ReloadWeapon();
}



void ULSEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void ULSEquipmentComponent::OnRepFocusEquipment(ALSWeaponBase* OldFocusEquipment)
{
	if (IsValid(OldFocusEquipment))
	{
		OldFocusEquipment->UnLinkWeaponAnimClassLayer();
	}

	// 새 포커스 무기 레이어 적용
	if (IsValid(FocusEquipment))
	{
		FocusEquipment->ApplyWeaponAnimLayer();
	}
}

void ULSEquipmentComponent::OnRepEquipments()
{
	UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent::OnRepEquipments()"));

	// Map 초기화
	Equipments.Reset();

	for (AActor* Equipment : ReplicatedEquipments)
	{
		ALSWeaponBase* WB = Cast<ALSWeaponBase>(Equipment);
		if (!WB)
		{
			continue;
		}

		const FWeaponData WeaponData = WB->GetWeaponData();
		
		// Map에 추가
		Equipments.Add(WeaponData.WeaponType, Equipment);
	}

	OnEquipmentArrayUpdated.Broadcast();
}


void ULSEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULSEquipmentComponent, FocusEquipment);
	DOREPLIFETIME(ULSEquipmentComponent, ReplicatedEquipments);
}

void ULSEquipmentComponent::AimRelease()
{
	if (FocusEquipment)
	{
		FocusEquipment->AimRelease();
	}
}

const TMap<EEquipmentType, TObjectPtr<AActor>> ULSEquipmentComponent::GetEquipments()
{
	return Equipments;
}

void ULSEquipmentComponent::ServerRPCEquipItemFromInventory_Implementation(FName ItemName)
{
	UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent: ServerRPCEquipItemFromInventory"));

	EquipItemFromInventory(ItemName);
}

void ULSEquipmentComponent::ServerRPCUnEquipItemFromInventory_Implementation(FName ItemName)
{
	ULSDataSubsystem* Sub = GetOwner()->GetGameInstance()->GetSubsystem<ULSDataSubsystem>();
	if (!Sub)
	{
		return;
	}

	const FWeaponData* ID = Sub->FindWeapon(ItemName);
	if (!ID)
	{
		return;
	}

	UnEquipItemFromInventory(ID->WeaponType);
}

void ULSEquipmentComponent::Aim()
{
	if (FocusEquipment)
	{
		FocusEquipment->Aim();
	}
}