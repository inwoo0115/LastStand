// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LSWeaponInfoData.generated.h"

class UAnimMontage;

UENUM(BlueprintType)
enum class EWeaponMontageType : uint8
{
	Fire      UMETA(DisplayName = "Fire"),
	Reload    UMETA(DisplayName = "Reload"),
	Equip     UMETA(DisplayName = "Equip")
};

/**
 *
 */
UCLASS()
class LASTSTAND_API ULSWeaponInfoData : public UDataAsset
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere, Category = SpringArm)
	float TargetArmLength = 40.0f;

	UPROPERTY(EditAnywhere, Category = WeaponInfo)
	uint32 MaxAmmo;

	UPROPERTY(EditAnywhere, Category = WeaponInfo)
	uint32 CurrentAmmo;

	UPROPERTY(EditAnywhere, Category = WeaponInfo)
	uint32 MaxRange;

	UPROPERTY(EditAnywhere, Category = WeaponInfo)
	float ShotGroupRadius;

	UPROPERTY(EditAnywhere, Category = WeaponInfo)
	float LaunchIntervalTime;

	UPROPERTY(EditAnywhere, Category = WeaponInfo)
	float ReloadIntervalTime;

	UPROPERTY(EditAnywhere, Category = WeaponInfo)
	float EquipIntervalTime;

	UPROPERTY(EditAnywhere, Category = WeaponInfo)
	bool bIsRapidFire;

	UPROPERTY(EditAnywhere, Category = WeaponInfo)
	FName MuzzleName;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSoftClassPtr<UAnimInstance> AnimLayerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TMap<EWeaponMontageType, TSoftObjectPtr<UAnimMontage>> WeaponMontages;
};
