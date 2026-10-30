// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/LSProjectileBase.h"
#include "Engine/NetSerialization.h"
#include "LSProjectileMissile.generated.h"

// 유도 미사일: 발사 방향으로 StraightDuration초 직진 → 그 시점 타깃 위치를 고정 → 고정 지점으로 선회 유도
// 잠금은 서버 권위(복제), 선회 계산은 모든 머신에서 동일하게 수행 (오차는 movement 리플리케이션이 보정)
UCLASS()
class LASTSTAND_API ALSProjectileMissile : public ALSProjectileBase
{
	GENERATED_BODY()

public:
	ALSProjectileMissile();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

	// 스폰 직후(FinishSpawning 전) 액션이 호출 — 서버
	void InitMissile(AActor* InTarget, float InStraightDuration, bool bInDrawDebug, float InDebugDrawDuration);

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	// 유도 확정: 타깃 위치 고정 (서버)
	void LockTarget();

	// 고정 지점으로 속도 방향 선회 (모든 머신)
	void UpdateHoming(float DeltaTime);

	UFUNCTION()
	void OnRep_HomingLocked();

	// 착탄 예정 지점 디버그
	void DrawDebugLockedLocation() const;

	// 타깃 발밑 바닥 지점 (아래로 트레이스, 실패 시 타깃 위치)
	FVector FindGroundLocation(const AActor* Target) const;

	// 초당 최대 선회 각도(도)
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Missile", meta = (ClampMin = "0.0"))
	float HomingTurnRate = 360.0f;

	// 타깃 위치에서 바닥을 찾는 하향 트레이스 거리
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Missile", meta = (ClampMin = "0.0"))
	float GroundTraceDistance = 1000.0f;

	// 바닥 탐색 채널
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Missile")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel = ECC_Visibility;

	// --- 액션이 주입 (초기 1회 복제) ---
	UPROPERTY(Replicated)
	float StraightDuration = 0.5f;

	UPROPERTY(Replicated)
	bool bDrawDebug = false;

	UPROPERTY(Replicated)
	float DebugDrawDuration = 1.0f;

	// --- 유도 상태 ---
	UPROPERTY(Replicated)
	FVector_NetQuantize LockedTargetLocation;

	UPROPERTY(ReplicatedUsing = OnRep_HomingLocked)
	bool bHomingLocked = false;

	// 고정 지점을 지나침 → 이후 직진 (로컬)
	bool bHomingFinished = false;

	// 잠금 대상 (서버)
	TWeakObjectPtr<AActor> TargetActor;

	// 경로 디버그 직전 위치
	FVector LastDebugLocation = FVector::ZeroVector;
};
