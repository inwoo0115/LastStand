// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Action/LSAIAction_MeleeSweep.h"
#include "AI/LSEnemyBase.h"
#include "AI/Components/LSAIActionComponent.h"
#include "Interface/LSStatComponentInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

bool ULSAIAction_MeleeSweep::OnActivate()
{
	bHitWindowOpen = false;
	HitActors.Reset();

	return Super::OnActivate();
}

void ULSAIAction_MeleeSweep::OnEnd(bool bSucceeded)
{
	bHitWindowOpen = false;
	HitActors.Reset();

	Super::OnEnd(bSucceeded);
}

void ULSAIAction_MeleeSweep::OnHitWindow(FName WindowName, bool bBegin)
{
	if (bBegin)
	{
		bHitWindowOpen = GetTraceLocation(LastTraceLocation);
		if (bHitWindowOpen)
		{
			SweepAndApplyDamage();
		}
	}
	else
	{
		bHitWindowOpen = false;
	}
}

void ULSAIAction_MeleeSweep::TickAction(float DeltaTime)
{
	if (bHitWindowOpen)
	{
		SweepAndApplyDamage();
	}
}

bool ULSAIAction_MeleeSweep::GetTraceLocation(FVector& OutLocation) const
{
	const ACharacter* OwnerCh = GetOwnerCharacter();
	if (!OwnerCh || !OwnerCh->GetMesh())
	{
		return false;
	}

	OutLocation = OwnerCh->GetMesh()->GetSocketLocation(TraceSocket);
	return true;
}

void ULSAIAction_MeleeSweep::SweepAndApplyDamage()
{
	UWorld* World = GetWorld();
	ACharacter* OwnerCh = GetOwnerCharacter();
	FVector CurrentLocation;
	if (!World || !OwnerCh || !GetTraceLocation(CurrentLocation))
	{
		return;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AIMeleeSweep), false, OwnerCh);
	TArray<FHitResult> Hits;
	World->SweepMultiByObjectType(Hits, LastTraceLocation, CurrentLocation, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(TraceRadius), Params);

	const FVector SweepStart = LastTraceLocation;
	LastTraceLocation = CurrentLocation;

	bool bAnyDamaged = false;
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();

		// 적끼리는 피해 없음, 같은 대상은 1회만
		if (!HitActor || HitActor->IsA<ALSEnemyBase>() || HitActors.Contains(HitActor))
		{
			continue;
		}

		if (HitActor->Implements<ULSStatComponentInterface>())
		{
			HitActors.Add(HitActor);
			Cast<ILSStatComponentInterface>(HitActor)->ApplyDamage(GetDamage(), OwnerCh);
			bAnyDamaged = true;
		}
	}

	// 스윕 궤적 디버그 (이번 스윕에서 데미지를 줬으면 빨강)
	if (bDrawDebug && OwnerComp)
	{
		OwnerComp->DrawDebugSweep(SweepStart, CurrentLocation, TraceRadius, bAnyDamaged, DebugDrawDuration);
	}
}
