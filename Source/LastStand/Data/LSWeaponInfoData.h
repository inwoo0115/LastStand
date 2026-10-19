// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LSWeaponInfoData.generated.h"

class UAnimMontage;
class USoundBase;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EWeaponMontageType : uint8
{
	Fire      UMETA(DisplayName = "Fire"),
	Reload    UMETA(DisplayName = "Reload"),
	Equip     UMETA(DisplayName = "Equip")
};

// 무기 애님 노티파이가 무기에 넘기는 재생 정보 (노티파이에 직접 지정)
USTRUCT(BlueprintType)
struct FWeaponNotifyEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TObjectPtr<UNiagaraSystem> NiagaraEffect;

	// 무기 메시 소켓 (None이면 메시 루트)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FName SocketName = NAME_None;
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
	int32 Damage;

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
