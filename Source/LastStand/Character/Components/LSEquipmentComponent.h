// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataTable/LSItemData.h"
#include "DataTable/LSWeaponData.h"
#include "LSEquipmentComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnEquipmentArrayUpdated);

// 탄알 캐시 갱신 시 발화 (향후 HUD 연동)
DECLARE_MULTICAST_DELEGATE(FOnAmmoCacheUpdated);

// 인벤토리에 보유한 탄알 캐시 엔트리 (AmmoID → 보유 수량)
USTRUCT()
struct FLSAmmoCacheEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FName AmmoID;

	UPROPERTY()
	int32 Count = 0;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTSTAND_API ULSEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULSEquipmentComponent();

	void EquipItemFromInventory(FName ItemName);

	void UnEquipItemFromInventory(EEquipmentType EquipType);

	void FocusEquipmentByType(EEquipmentType EquipType);

	void LaunchEquipment();

	void ReleaseEquipment();

	void Reload();

	void Aim();

	void AimRelease();

	FOnEquipmentArrayUpdated OnEquipmentArrayUpdated;

	FOnAmmoCacheUpdated OnAmmoCacheUpdated;

	const TMap<EEquipmentType, TObjectPtr<AActor>> GetEquipments();

	// 현재 포커스 무기의 타입 (없으면 None) — HUD 하이라이트용
	EEquipmentType GetFocusEquipmentType() const;

	// 현재 포커스 무기 (없으면 nullptr) — HUD pull용
	class ALSWeaponBase* GetFocusEquipment() const { return FocusEquipment; }

	// 지정 탄약 ID의 캐시 보유량 (없으면 0) — HUD pull용
	int32 GetAmmoCount(FName AmmoName) const;

	// Server RPC
	UFUNCTION(Server, Reliable)
	void ServerRPCEquipItemFromInventory(FName ItemName);

	UFUNCTION(Server, Reliable)
	void ServerRPCUnEquipItemFromInventory(FName ItemName);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 인벤토리 수량 변경 델리게이트 핸들러 (서버) — 탄알이면 캐시에 델타 반영
	void HandleInventoryItemChanged(FName ItemID, int32 Delta);

	// 탄알 캐시 델타 적용 (서버 권위) — Delta 부호 있음, 0 이하가 되면 엔트리 제거
	void ApplyAmmoDelta(FName AmmoID, int32 Delta);

	// 캐시 갱신 후 컴포넌트 델리게이트 + 로컬 플레이어 HUD로 전파
	void BroadcastAmmoCacheToUI();

	UFUNCTION()
	void OnRep_AmmoCache();

	UFUNCTION()
	void OnRepFocusEquipment(class ALSWeaponBase* OldFocusEquipment);

	UFUNCTION()
	void OnRepEquipments();

	// 로컬 플레이어 소유 폰일 때만 UI 서브시스템 반환 (데디 서버·원격·AI 배제)
	class ULSUIEventSubsystem* GetLocalPlayerUISubsystem() const;

	// RPC
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 리플리케이션용 배열
	UPROPERTY(ReplicatedUsing = OnRepEquipments)
	TArray<TObjectPtr<AActor>> ReplicatedEquipments;

	// 장착 무기
	UPROPERTY(EditAnywhere, Category = Equipment, Meta = (AllowPrivateAccess = "true"))
	TMap<EEquipmentType, TObjectPtr<AActor>> Equipments;

	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRepFocusEquipment)
	TObjectPtr<class ALSWeaponBase> FocusEquipment;

	// 인벤토리 보유 탄알 캐시 (서버에서 갱신, 클라로 리플리케이트)
	UPROPERTY(ReplicatedUsing = OnRep_AmmoCache)
	TArray<FLSAmmoCacheEntry> AmmoCache;
};
