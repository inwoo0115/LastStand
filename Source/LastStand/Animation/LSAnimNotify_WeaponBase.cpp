// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LSAnimNotify_WeaponBase.h"
#include "Character/Components/LSEquipmentComponent.h"
#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Sound/SoundBase.h"

void ULSAnimNotify_WeaponBase::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// 에디터 프리뷰 등 폰/장비가 없으면 무시
	APawn* OwnerPawn = MeshComp ? Cast<APawn>(MeshComp->GetOwner()) : nullptr;
	if (!OwnerPawn || !ShouldExecute(OwnerPawn))
	{
		return;
	}

	const ULSEquipmentComponent* EC = OwnerPawn->GetComponentByClass<ULSEquipmentComponent>();
	ALSWeaponBase* Weapon = EC ? EC->GetFocusEquipment() : nullptr;
	if (!Weapon)
	{
		return;
	}

	ExecuteOnWeapon(Weapon);
}

FString ULSAnimNotify_WeaponBase::GetNotifyName_Implementation() const
{
	return Effect.Sound ? FString::Printf(TEXT("%s: %s"), *GetNotifyPrefix(), *Effect.Sound->GetName()) : GetNotifyPrefix();
}
