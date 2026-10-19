// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/LSAnimNotify_WeaponBase.h"
#include "LSAnimNotify_WeaponReload.generated.h"

// 재장전 몽타주 노티파이. 소유 로컬 플레이어 클라에서만 실행 (몽타주는 타 머신에도 재생되지만 무시)
UCLASS(meta = (DisplayName = "LS Weapon Reload"))
class LASTSTAND_API ULSAnimNotify_WeaponReload : public ULSAnimNotify_WeaponBase
{
	GENERATED_BODY()

protected:
	virtual bool ShouldExecute(const APawn* OwnerPawn) const override;

	virtual void ExecuteOnWeapon(ALSWeaponBase* Weapon) const override;

	virtual FString GetNotifyPrefix() const override { return TEXT("Reload"); }
};
