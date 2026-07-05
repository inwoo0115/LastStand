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

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRepFocusEquipment();

	UFUNCTION()
	void OnRepEquipments();

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
