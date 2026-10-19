// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/LSAnimNotify_WeaponBase.h"
#include "LSAnimNotify_WeaponFire.generated.h"

// 발사 몽타주 노티파이. 데디 서버 제외 몽타주가 재생되는 모든 머신에서 실행
UCLASS(meta = (DisplayName = "LS Weapon Fire"))
class LASTSTAND_API ULSAnimNotify_WeaponFire : public ULSAnimNotify_WeaponBase
{
	GENERATED_BODY()

protected:
	virtual bool ShouldExecute(const APawn* OwnerPawn) const override;

	virtual void ExecuteOnWeapon(ALSWeaponBase* Weapon) const override;

	virtual FString GetNotifyPrefix() const override { return TEXT("Fire"); }
};
