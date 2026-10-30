// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/LSProjectileMissile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ALSProjectileMissile::ALSProjectileMissile()
{
	// 유도/경로 디버그용 틱
	PrimaryActorTick.bCanEverTick = true;
}

void ALSProjectileMissile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 스폰 시 고정되는 설정값
	DOREPLIFETIME_CONDITION(ALSProjectileMissile, StraightDuration, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ALSProjectileMissile, bDrawDebug, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ALSProjectileMissile, DebugDrawDuration, COND_InitialOnly);

	DOREPLIFETIME(ALSProjectileMissile, LockedTargetLocation);
	DOREPLIFETIME(ALSProjectileMissile, bHomingLocked);
}

void ALSProjectileMissile::InitMissile(AActor* InTarget, float InStraightDuration, bool bInDrawDebug, float InDebugDrawDuration)
{
	TargetActor = InTarget;
	StraightDuration = InStraightDuration;
	bDrawDebug = bInDrawDebug;
	DebugDrawDuration = InDebugDrawDuration;
}

void ALSProjectileMissile::BeginPlay()
{
	Super::BeginPlay();

	LastDebugLocation = GetActorLocation();
}

void ALSProjectileMissile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 직진 구간 종료 → 유도 확정 (서버)
	if (HasAuthority() && !bHomingLocked && GetGameTimeSinceCreation() >= StraightDuration)
	{
		LockTarget();
	}

	// 타깃 없이 잠긴 경우(LockedTargetLocation 미설정)는 클라에서도 직진 유지
	if (bHomingLocked && !bHomingFinished && !LockedTargetLocation.IsZero())
	{
		UpdateHoming(DeltaTime);
	}

#if ENABLE_DRAW_DEBUG
	// 경로 디버그: 직진 초록 / 유도 빨강
	if (bDrawDebug && GetNetMode() != NM_DedicatedServer)
	{
		const FVector CurrentLocation = GetActorLocation();
		const FColor PathColor = (bHomingLocked && !bHomingFinished) ? FColor::Red : FColor::Green;
		DrawDebugLine(GetWorld(), LastDebugLocation, CurrentLocation, PathColor, false, DebugDrawDuration, 0, 2.0f);
		LastDebugLocation = CurrentLocation;
	}
#endif
}

void ALSProjectileMissile::LockTarget()
{
	const AActor* Target = TargetActor.Get();
	if (!Target)
	{
		// 타깃 없음 → 계속 직진 (매 틱 재시도 방지)
		bHomingFinished = true;
		bHomingLocked = true;
		return;
	}

	// 이후 타깃이 움직여도 따라가지 않음 — 타깃 발밑 바닥으로 고정
	LockedTargetLocation = FindGroundLocation(Target);
	bHomingLocked = true;

	DrawDebugLockedLocation();
}

FVector ALSProjectileMissile::FindGroundLocation(const AActor* Target) const
{
	const FVector Start = Target->GetActorLocation();
	const FVector End = Start - FVector(0.0f, 0.0f, GroundTraceDistance);

	// 타깃/미사일/발사자는 무시 → 바닥에만 맞도록
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MissileGroundTrace), false, this);
	Params.AddIgnoredActor(Target);
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(GetInstigator());

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, GroundTraceChannel, Params))
	{
		return Hit.ImpactPoint;
	}

	// 바닥 없음(공중 등) → 타깃 위치
	return Start;
}

void ALSProjectileMissile::UpdateHoming(float DeltaTime)
{
	const FVector Velocity = ProjectileMovement->Velocity;
	const FVector ToTarget = FVector(LockedTargetLocation) - GetActorLocation();

	// 고정 지점을 지나쳤으면 유도 종료
	if (ToTarget.IsNearlyZero())
	{
		bHomingFinished = true;
		return;
	}

	// 속력 유지, 방향만 초당 HomingTurnRate도까지 선회
	const FVector NewDir = FMath::VInterpNormalRotationTo(Velocity.GetSafeNormal(), ToTarget.GetSafeNormal(), DeltaTime, HomingTurnRate);
	ProjectileMovement->Velocity = NewDir * Velocity.Size();
}

void ALSProjectileMissile::OnRep_HomingLocked()
{
	DrawDebugLockedLocation();
}

void ALSProjectileMissile::DrawDebugLockedLocation() const
{
#if ENABLE_DRAW_DEBUG
	// 데디 서버는 뷰포트 없음 → 스킵. 타깃 없이 잠긴 경우(직진 유지)는 표시하지 않음
	if (!bDrawDebug || GetNetMode() == NM_DedicatedServer || LockedTargetLocation.IsZero())
	{
		return;
	}

	// 착탄 예정 지점 (노랑)
	DrawDebugSphere(GetWorld(), LockedTargetLocation, 40.0f, 12, FColor::Yellow, false, DebugDrawDuration, 0, 2.0f);
#endif
}

void ALSProjectileMissile::Destroyed()
{
#if ENABLE_DRAW_DEBUG
	// 실제 착탄 지점 (빨강)
	if (bDrawDebug && GetNetMode() != NM_DedicatedServer && GetWorld() && GetWorld()->IsGameWorld())
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), 30.0f, 12, FColor::Red, false, DebugDrawDuration, 0, 2.0f);
	}
#endif

	Super::Destroyed();
}
