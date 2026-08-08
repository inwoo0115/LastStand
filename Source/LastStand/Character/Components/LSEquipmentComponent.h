// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataTable/LSItemData.h"
#include "DataTable/LSWeaponData.h"
#include "LSEquipmentComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnEquipmentArrayUpdated);


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

	const TMap<EEquipmentType, TObjectPtr<AActor>> GetEquipments();

	// 현재 포커스 무기의 타입 (없으면 None) — HUD 하이라이트용
	EEquipmentType GetFocusEquipmentType() const;

	// Server RPC
	UFUNCTION(Server, Reliable)
	void ServerRPCEquipItemFromInventory(FName ItemName);

	UFUNCTION(Server, Reliable)
	void ServerRPCUnEquipItemFromInventory(FName ItemName);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

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
};
