// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Action/LSAIAction_SpawnMissile.h"
#include "Projectile/LSProjectileMissile.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

bool ULSAIAction_SpawnMissile::OnActivate()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	LoadedProjectileClass = ProjectileClass.LoadSynchronous();
	if (!LoadedProjectileClass)
	{
		return false;
	}

	// 몽타주는 선택 — 없거나 재생 실패해도 발사는 진행
	if (!Montage.IsNull())
	{
		StartMontage();
	}

	FiredCount = 0;
	NextFireTime = World->GetTimeSeconds() + FirstFireDelay;

	return true;
}

void ULSAIAction_SpawnMissile::TickAction(float DeltaTime)
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
		SpawnMissile();
		++FiredCount;
		NextFireTime = Now + FireInterval;
	}

	if (FiredCount >= MissileCount)
	{
		FinishAction(true);
	}
}

void ULSAIAction_SpawnMissile::SpawnMissile()
{
	UWorld* World = GetWorld();
	ACharacter* OwnerCh = GetOwnerCharacter();
	if (!World || !OwnerCh || !LoadedProjectileClass)
	{
		return;
	}

	const FTransform LaunchTransform = GetLaunchTransform();

	// 지연 스폰 → 초기 복제 전에 타깃/설정 주입
	ALSProjectileMissile* Missile = World->SpawnActorDeferred<ALSProjectileMissile>(
		LoadedProjectileClass,
		LaunchTransform,
		OwnerCh,
		OwnerCh,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Missile)
	{
		return;
	}

	Missile->InitMissile(GetTargetActor(), StraightDuration, bDrawDebug, DebugDrawDuration);
	Missile->FinishSpawning(LaunchTransform);
}

FTransform ULSAIAction_SpawnMissile::GetLaunchTransform() const
{
	const ACharacter* OwnerCh = GetOwnerCharacter();
	const USkeletalMeshComponent* Mesh = OwnerCh ? OwnerCh->GetMesh() : nullptr;
	if (!Mesh)
	{
		return OwnerCh ? OwnerCh->GetActorTransform() : FTransform::Identity;
	}

	if (LaunchSockets.IsEmpty())
	{
		return FTransform(OwnerCh->GetActorRotation(), Mesh->GetComponentLocation());
	}

	// 소켓 위치 + 소켓 X축 방향 (스케일 제외)
	const FName Socket = LaunchSockets[FiredCount % LaunchSockets.Num()];
	const FTransform SocketTransform = Mesh->GetSocketTransform(Socket);
	return FTransform(SocketTransform.GetRotation(), SocketTransform.GetLocation());
}
