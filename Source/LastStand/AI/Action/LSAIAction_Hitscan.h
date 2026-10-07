// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Action/LSAIActionBase.h"
#include "LSAIAction_Hitscan.generated.h"

// 원거리 공격: 상태 유지 중 FireInterval마다 타깃 중심 방향을 축으로 한 원뿔 범위 안에 랜덤 탄퍼짐 히트스캔 (펠릿마다 데미지)
// 스스로 끝나지 않음 → StateTree 상태 이탈(Cancel)로만 종료. 몽타주는 선택(루프 사격 몽타주 권장)
UCLASS(meta = (DisplayName = "Hitscan"))
class LASTSTAND_API ULSAIAction_Hitscan : public ULSAIActionBase
{
	GENERATED_BODY()

public:
	virtual void TickAction(float DeltaTime) override;

protected:
	virtual bool OnActivate() override;

	// 몽타주가 끝나거나 끊겨도 발사 유지
	virtual void OnMontageEnded(bool bInterrupted) override {}

	// 1회 발사 (PelletCount발)
	void Fire();

	// 발사 간격(초)
	UPROPERTY(EditAnywhere, Category = "Hitscan", meta = (ClampMin = "0.05"))
	float FireInterval = 0.5f;

	// 진입 후 첫 발까지 지연(초)
	UPROPERTY(EditAnywhere, Category = "Hitscan", meta = (ClampMin = "0.0"))
	float FirstFireDelay = 0.0f;

	// 발사 기준 메시 소켓 (None이면 메시 원점)
	UPROPERTY(EditAnywhere, Category = "Hitscan")
	FName MuzzleSocket = NAME_None;

	// 탄퍼짐 원뿔 반각(도)
	UPROPERTY(EditAnywhere, Category = "Hitscan", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float ConeHalfAngle = 5.0f;

	// 1회 발사당 탄 수 (1 = 단발, 여러 개 = 산탄)
	UPROPERTY(EditAnywhere, Category = "Hitscan", meta = (ClampMin = "1"))
	int32 PelletCount = 1;

	UPROPERTY(EditAnywhere, Category = "Hitscan", meta = (ClampMin = "0.0"))
	float Range = 5000.0f;

	// 플레이어 캡슐(Pawn 프로파일)이 블록하는 채널
	UPROPERTY(EditAnywhere, Category = "Hitscan")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	// 다음 발사 시각 (월드 시간)
	float NextFireTime = 0.0f;

	// 타깃을 가지고 시작했는지 (시작 타깃 고정 — 무효화되면 발사 스킵)
	bool bStartedWithTarget = false;
};
