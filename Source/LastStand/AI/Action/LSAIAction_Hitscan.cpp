// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Action/LSAIAction_Hitscan.h"
#include "AI/LSEnemyBase.h"
#include "AI/Components/LSAIActionComponent.h"
#include "Interface/LSStatComponentInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

bool ULSAIAction_Hitscan::OnActivate()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// 몽타주는 선택 — 없거나 재생 실패해도 발사는 진행
	if (!Montage.IsNull())
	{
		StartMontage();
	}

	bStartedWithTarget = GetTargetActor() != nullptr;
	NextFireTime = World->GetTimeSeconds() + FirstFireDelay;

	return true;
}

void ULSAIAction_Hitscan::TickAction(float DeltaTime)
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 틱당 최대 1발 (프레임 드랍 시 몰아 쏘기 방지)
	const float Now = World->GetTimeSeconds();
	if (Now >= NextFireTime)
	{
		Fire();
		NextFireTime = Now + FireInterval;
	}
}

void ULSAIAction_Hitscan::Fire()
{
	UWorld* World = GetWorld();
	ACharacter* OwnerCh = GetOwnerCharacter();
	if (!World || !OwnerCh || !OwnerCh->GetMesh())
	{
		return;
	}

	// 시작 타깃이 사라졌으면(사망/파괴) 이번 발 스킵
	const AActor* Target = GetTargetActor();
	if (bStartedWithTarget && !Target)
	{
		return;
	}

	// 축: 발사 위치 → 타깃 중심 (타깃 없이 시작했으면 정면)
	const FVector Start = OwnerCh->GetMesh()->GetSocketLocation(MuzzleSocket);
	FVector Axis = Target ? (Target->GetActorLocation() - Start).GetSafeNormal() : FVector::ZeroVector;
	if (Axis.IsNearlyZero())
	{
		Axis = OwnerCh->GetActorForwardVector();
	}

	const float HalfAngleRad = FMath::DegreesToRadians(ConeHalfAngle);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AIHitscan), false, OwnerCh);

	TArray<FVector_NetQuantize> DebugEnds;
	TArray<bool> DebugHits;

	for (int32 Pellet = 0; Pellet < PelletCount; ++Pellet)
	{
		// 원뿔 안 랜덤 방향
		const FVector Dir = HalfAngleRad > 0.0f ? FMath::VRandCone(Axis, HalfAngleRad) : Axis;
		const FVector End = Start + Dir * Range;

		FHitResult Hit;
		const bool bBlocked = World->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, Params);

		bool bDamaged = false;
		AActor* HitActor = bBlocked ? Hit.GetActor() : nullptr;

		// 적끼리는 피해 없음. 펠릿마다 데미지 누적 (산탄)
		if (HitActor && !HitActor->IsA<ALSEnemyBase>() && HitActor->Implements<ULSStatComponentInterface>())
		{
			Cast<ILSStatComponentInterface>(HitActor)->ApplyDamage(GetDamage(), OwnerCh);
			bDamaged = true;
		}

		if (bDrawDebug)
		{
			DebugEnds.Add(bBlocked ? Hit.ImpactPoint : End);
			DebugHits.Add(bDamaged);
		}
	}

	// 원뿔 + 탄도 디버그 (전 머신)
	if (bDrawDebug && OwnerComp)
	{
		OwnerComp->DrawDebugHitscan(Start, Axis, Range, ConeHalfAngle, DebugEnds, DebugHits, DebugDrawDuration);
	}
}
