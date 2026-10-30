// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Action/LSAIActionBase.h"
#include "LSAIAction_SpawnMissile.generated.h"

class ALSProjectileMissile;

// 메시 소켓에서 유도 미사일을 MissileCount발, FireInterval 간격으로 발사. 마지막 발사 후 성공 종료
// 몽타주는 선택(재생해도 발사 흐름과 무관). 중단 시 남은 발사만 취소, 발사된 미사일은 계속 비행
UCLASS(meta = (DisplayName = "Spawn Missile"))
class LASTSTAND_API ULSAIAction_SpawnMissile : public ULSAIActionBase
{
	GENERATED_BODY()

public:
	virtual void TickAction(float DeltaTime) override;

protected:
	virtual bool OnActivate() override;

	// 몽타주가 먼저 끝나거나 끊겨도 발사 유지
	virtual void OnMontageEnded(bool bInterrupted) override {}

	// 1발 스폰 (FiredCount 기준 소켓 순환)
	void SpawnMissile();

	// 발사 위치/방향 (소켓 X축). 소켓 없으면 메시 원점·액터 정면
	FTransform GetLaunchTransform() const;

	UPROPERTY(EditAnywhere, Category = "Missile")
	TSoftClassPtr<ALSProjectileMissile> ProjectileClass;

	// 발사 소켓. 여러 개면 발사마다 순환
	UPROPERTY(EditAnywhere, Category = "Missile")
	TArray<FName> LaunchSockets;

	UPROPERTY(EditAnywhere, Category = "Missile", meta = (ClampMin = "1"))
	int32 MissileCount = 1;

	// 발사 간격(초)
	UPROPERTY(EditAnywhere, Category = "Missile", meta = (ClampMin = "0.0"))
	float FireInterval = 0.3f;

	// 진입 후 첫 발까지 지연(초)
	UPROPERTY(EditAnywhere, Category = "Missile", meta = (ClampMin = "0.0"))
	float FirstFireDelay = 0.0f;

	// 발사 방향 직진 시간(초) → 이후 타깃 위치 고정 후 유도
	UPROPERTY(EditAnywhere, Category = "Missile", meta = (ClampMin = "0.0"))
	float StraightDuration = 0.5f;

	UPROPERTY(Transient)
	TSubclassOf<ALSProjectileMissile> LoadedProjectileClass;

	// 다음 발사 시각 (월드 시간)
	float NextFireTime = 0.0f;

	int32 FiredCount = 0;
};
