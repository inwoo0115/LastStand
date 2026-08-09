// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSHitboxComponent.h"

ULSHitboxComponent::ULSHitboxComponent()
{
	// 무기 히트스캔(ECC_Visibility)에만 반응하는 쿼리 전용 콜리전
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SetGenerateOverlapEvents(false);
}

void ULSHitboxComponent::SetupHitbox(ELSHitboxType InType, float InMultiplier)
{
	HitboxType = InType;
	DamageMultiplier = InMultiplier;
}
