// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LSAnimNotify_WeaponFire.h"
#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "GameFramework/Pawn.h"

bool ULSAnimNotify_WeaponFire::ShouldExecute(const APawn* OwnerPawn) const
{
	// 데디 서버는 렌더링/사운드가 없으므로 스킵
	return OwnerPawn->GetNetMode() != NM_DedicatedServer;
}

void ULSAnimNotify_WeaponFire::ExecuteOnWeapon(ALSWeaponBase* Weapon) const
{
	Weapon->OnFireNotify(Effect);
}
