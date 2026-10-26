// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSActionStateComponent.h"
#include "Character/Components/LSCharacterMovementComponent.h"
#include "Character/Components/LSEquipmentComponent.h"
#include "Item/Equipment/Weapon/LSWeaponBase.h"
#include "GameFramework/Character.h"
#include "Tags/LSGameplayTags.h"

ULSActionStateComponent::ULSActionStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool ULSActionStateComponent::CanPerformAction(const FGameplayTag& Action) const
{
	const FLSActionRule* Rule = ActionRules.Find(Action);
	if (!Rule)
	{
		return true;
	}

	return !GetActiveStates().HasAny(Rule->BlockedByStates);
}

void ULSActionStateComponent::CancelStatesFor(const FGameplayTag& Action)
{
	const FLSActionRule* Rule = ActionRules.Find(Action);
	if (!Rule)
	{
		return;
	}

	// 상태별 취소 처리 — 새 취소 가능 상태는 여기에 분기 추가
	if (Rule->CancelStates.HasTag(LSStateTags::State_Reloading))
	{
		if (ALSWeaponBase* Weapon = GetFocusWeapon())
		{
			Weapon->CancelReload();
		}
	}
}

FGameplayTagContainer ULSActionStateComponent::GetActiveStates() const
{
	FGameplayTagContainer States = LooseStates;

	// 대시: 루트모션 존재 여부 (서버/소유 클라 각자 예측 적용 → 각 머신 값 그대로 사용)
	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (const ULSCharacterMovementComponent* CMC = Cast<ULSCharacterMovementComponent>(Character->GetCharacterMovement()))
		{
			if (CMC->IsDashing())
			{
				States.AddTag(LSStateTags::State_Dashing);
			}
		}
	}

	// 무기: 장전/장착 중
	if (const ALSWeaponBase* Weapon = GetFocusWeapon())
	{
		if (Weapon->IsReloading())
		{
			States.AddTag(LSStateTags::State_Reloading);
		}
		if (Weapon->IsEquipping())
		{
			States.AddTag(LSStateTags::State_Equipping);
		}
	}

	return States;
}

ALSWeaponBase* ULSActionStateComponent::GetFocusWeapon() const
{
	const ULSEquipmentComponent* EC = GetOwner() ? GetOwner()->GetComponentByClass<ULSEquipmentComponent>() : nullptr;
	return EC ? EC->GetFocusEquipment() : nullptr;
}
