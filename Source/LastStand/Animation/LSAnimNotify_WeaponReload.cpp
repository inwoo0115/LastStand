// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LSAnimNotify_WeaponReload.h"
#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "GameFramework/Pawn.h"

bool ULSAnimNotify_WeaponReload::ShouldExecute(const APawn* OwnerPawn) const
{
	// 소유 로컬 플레이어만 (원격 클라/서버의 시뮬레이티드 폰 제외)
	return OwnerPawn->IsLocallyControlled() && OwnerPawn->IsPlayerControlled();
}

void ULSAnimNotify_WeaponReload::ExecuteOnWeapon(ALSWeaponBase* Weapon) const
{
	Weapon->OnReloadNotify(Effect);
}
