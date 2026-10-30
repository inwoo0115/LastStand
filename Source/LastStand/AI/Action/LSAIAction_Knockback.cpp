// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Action/LSAIAction_Knockback.h"
#include "AI/LSEnemyBase.h"
#include "AI/Components/LSAIActionComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

void ULSAIAction_Knockback::OnHitWindow(FName WindowName, bool bBegin)
{
	// 구간 시작에만 1회 적용
	if (!bBegin)
	{
		return;
	}

	if (!KnockbackWindowName.IsNone() && WindowName != KnockbackWindowName)
	{
		return;
	}

	ApplyKnockback();
}

void ULSAIAction_Knockback::ApplyKnockback()
{
	UWorld* World = GetWorld();
	ACharacter* OwnerCh = GetOwnerCharacter();
	if (!World || !OwnerCh)
	{
		return;
	}

	const FVector Center = OwnerCh->GetActorLocation();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AIKnockback), false, OwnerCh);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(KnockbackRadius), Params);

	// 한 액터가 여러 컴포넌트로 겹쳐도 1회만
	TSet<ACharacter*> Knocked;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		ACharacter* TargetCh = Cast<ACharacter>(Overlap.GetActor());

		// 적끼리는 넉백 없음
		if (!TargetCh || TargetCh->IsA<ALSEnemyBase>() || Knocked.Contains(TargetCh))
		{
			continue;
		}

		// 바깥쪽 수평 방향 (중심과 겹치면 대상 뒤쪽)
		FVector Dir = (TargetCh->GetActorLocation() - Center).GetSafeNormal2D();
		if (Dir.IsNearlyZero())
		{
			Dir = -TargetCh->GetActorForwardVector();
		}

		TargetCh->LaunchCharacter(Dir * KnockbackStrength + FVector::UpVector * KnockbackUpStrength, true, true);
		Knocked.Add(TargetCh);
	}

	// 넉백 범위 디버그 (대상이 있으면 빨강)
	if (bDrawDebug && OwnerComp)
	{
		OwnerComp->DrawDebugSweep(Center, Center, KnockbackRadius, Knocked.Num() > 0, DebugDrawDuration);
	}
}
