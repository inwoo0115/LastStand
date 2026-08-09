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
#include "GameFramework/Pawn.h"
#include "UI/LSUIEventSubsystem.h"
#include "Item/LSItemBase.h"
#include "Kismet/GameplayStatics.h"

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

	TSubclassOf<AActor> WeaponClass = ID->WeaponClass.LoadSynchronous();

	// 지연 스폰: BeginPlay(InitEquipment) 전에 ItemName을 주입해 액터가 올바른 데이터로 초기화되게 함
	const FTransform SpawnTransform(
		OwnerCharacter->GetMesh()->GetSocketRotation(ID->SocketName),
		OwnerCharacter->GetMesh()->GetSocketLocation(ID->SocketName));

	AActor* SpawnedActor = GetWorld()->SpawnActorDeferred<AActor>(
		WeaponClass, SpawnTransform, GetOwner(), nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	if (!SpawnedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnerComponent] 스폰 실패: %s"), *WeaponClass->GetName());
		return;
	}

	// 드롭한 아이템 이름 부여
	if (ALSItemBase* SpawnedItem = Cast<ALSItemBase>(SpawnedActor))
	{
		SpawnedItem->SetItemName(ItemName);
	}

	UGameplayStatics::FinishSpawningActor(SpawnedActor, SpawnTransform);

	// 소켓에 부착
	SpawnedActor->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		ID->SocketName
	);

	// 장비 타입 체크
	const EEquipmentType EquipType = ID->WeaponType;

	UE_LOG(LogTemp, Warning, TEXT("EquipItemFromInventory WeaponType: %s"), *UEnum::GetValueAsString(EquipType));


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

	if (EquipType == EEquipmentType::None || !Equipments.Contains(EquipType))
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

	// 서버에서만 인벤토리 습득 델리게이트 구독 (탄알 캐싱은 서버 권위)
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (ULSInventoryComponent* IC = GetOwner()->GetComponentByClass<ULSInventoryComponent>())
		{
			IC->OnInventoryItemChanged.AddUObject(this, &ULSEquipmentComponent::HandleInventoryItemChanged);
		}
	}
}

void ULSEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 구독 해제 (같은 액터에 붙은 인벤토리가 아직 살아있을 수 있음)
	if (GetOwner())
	{
		if (ULSInventoryComponent* IC = GetOwner()->GetComponentByClass<ULSInventoryComponent>())
		{
			IC->OnInventoryItemChanged.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ULSEquipmentComponent::HandleInventoryItemChanged(FName ItemID, int32 Delta)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	ULSDataSubsystem* Sub = GetOwner()->GetGameInstance()->GetSubsystem<ULSDataSubsystem>();
	if (!Sub)
	{
		return;
	}

	const FItemData* Row = Sub->FindItem(ItemID);
	if (!Row || Row->ItemType != EItemType::Ammo)
	{
		return;
	}

	ApplyAmmoDelta(ItemID, Delta);
}

void ULSEquipmentComponent::ApplyAmmoDelta(FName AmmoID, int32 Delta)
{
	// 기존 엔트리면 델타 반영 (음수면 차감), 0 이하가 되면 엔트리 제거
	for (int32 Index = 0; Index < AmmoCache.Num(); ++Index)
	{
		if (AmmoCache[Index].AmmoID == AmmoID)
		{
			AmmoCache[Index].Count += Delta;

			if (AmmoCache[Index].Count <= 0)
			{
				UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent::ApplyAmmoDelta 소진 제거: %s"), *AmmoID.ToString());
				AmmoCache.RemoveAtSwap(Index);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent::ApplyAmmoDelta 갱신: %s → %d (Δ%d)"), *AmmoID.ToString(), AmmoCache[Index].Count, Delta);
			}

			// authority는 OnRep이 호출되지 않으므로 직접 브로드캐스트
			BroadcastAmmoCacheToUI();
			return;
		}
	}

	// 엔트리가 없으면 양수 델타일 때만 신규 추가 (음수는 반영할 대상 없음)
	if (Delta <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent::ApplyAmmoDelta 무시(엔트리 없음, Δ%d): %s"), Delta, *AmmoID.ToString());
		return;
	}

	FLSAmmoCacheEntry NewEntry;
	NewEntry.AmmoID = AmmoID;
	NewEntry.Count = Delta;
	AmmoCache.Add(NewEntry);

	UE_LOG(LogTemp, Log, TEXT("ULSEquipmentComponent::ApplyAmmoDelta 신규: %s → %d"), *AmmoID.ToString(), Delta);

	BroadcastAmmoCacheToUI();
}

void ULSEquipmentComponent::BroadcastAmmoCacheToUI()
{
	OnAmmoCacheUpdated.Broadcast();

	// 로컬 플레이어 HUD에 예비 탄약 변경 전파 (데디 서버·원격·AI는 가드로 no-op)
	if (ULSUIEventSubsystem* UISub = GetLocalPlayerUISubsystem())
	{
		UISub->ReserveAmmoChanged.Broadcast();
	}
}

int32 ULSEquipmentComponent::GetAmmoCount(FName AmmoName) const
{
	for (const FLSAmmoCacheEntry& Entry : AmmoCache)
	{
		if (Entry.AmmoID == AmmoName)
		{
			return Entry.Count;
		}
	}
	return 0;
}

void ULSEquipmentComponent::OnRep_AmmoCache()
{
	BroadcastAmmoCacheToUI();
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

	// 로컬 플레이어 HUD에 포커스 변경 전파 (데디 서버는 가드로 no-op)
	if (ULSUIEventSubsystem* UISub = GetLocalPlayerUISubsystem())
	{
		UISub->FocusEquipmentChanged.Broadcast();
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
		UE_LOG(LogTemp, Warning, TEXT("OnRepEquipments() WeaponType: %s"), *UEnum::GetValueAsString(WeaponData.WeaponType));

		
		// Map에 추가
		Equipments.Add(WeaponData.WeaponType, Equipment);
	}

	OnEquipmentArrayUpdated.Broadcast();

	// 로컬 플레이어 HUD에 장비 배열 변경(최초 리플리케이션=초기화 포함) 전파 (데디 서버 no-op)
	if (ULSUIEventSubsystem* UISub = GetLocalPlayerUISubsystem())
	{
		UISub->EquipmentArrayChanged.Broadcast();
	}
}

EEquipmentType ULSEquipmentComponent::GetFocusEquipmentType() const
{
	return FocusEquipment ? FocusEquipment->GetWeaponData().WeaponType : EEquipmentType::None;
}

ULSUIEventSubsystem* ULSEquipmentComponent::GetLocalPlayerUISubsystem() const
{
	// 적(AI)·원격 플레이어·데디 서버 제외: 로컬 조종 + 플레이어 조종 폰만 (HUD 소유 클라)
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled() || !OwnerPawn->IsPlayerControlled())
	{
		return nullptr;
	}

	UGameInstance* GI = GetOwner()->GetGameInstance();
	return GI ? GI->GetSubsystem<ULSUIEventSubsystem>() : nullptr;
}


void ULSEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULSEquipmentComponent, FocusEquipment);
	DOREPLIFETIME(ULSEquipmentComponent, ReplicatedEquipments);
	DOREPLIFETIME(ULSEquipmentComponent, AmmoCache);
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