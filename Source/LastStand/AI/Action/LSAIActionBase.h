// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "LSAIActionBase.generated.h"

class ULSAIActionComponent;
class UAnimMontage;
class ACharacter;

// 적 AI 행동(공격 등) 하나의 정의 + 실행 로직 (서버 전용)
// 파라미터 프리셋은 BP 서브클래스 기본값으로 만들고 EnemyData 행에서 클래스로 참조 → ULSAIActionComponent가 적마다 생성해 실행
// 기본 동작: 몽타주 재생 → 몽타주 종료 시 성공 / 중단 시 실패
UCLASS(Abstract, Blueprintable)
class LASTSTAND_API ULSAIActionBase : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;

	// 컴포넌트가 생성 직후 호출 — 런타임 소유자/태그/기본 데미지 주입
	void InitializeAction(ULSAIActionComponent* InOwnerComp, FGameplayTag InActionTag, int32 InDefaultDamage);

	// 쿨다운·사거리 검사 (StateTree 조건에서 사용)
	virtual bool CanActivate(AActor* InTarget) const;

	// 실행 시작. 실패 시 false (활성 상태로 남지 않음)
	bool Activate(AActor* InTarget);

	// 외부 중단 (StateTree 상태 이탈 등) — 실패로 종료
	void Cancel();

	// 활성 중 매 틱 (컴포넌트 틱에서 호출, 서버)
	virtual void TickAction(float DeltaTime) {}

	// 애님 노티파이 스테이트의 히트 판정 구간 시작/끝 (서버)
	virtual void OnHitWindow(FName WindowName, bool bBegin) {}

	bool IsActive() const { return bIsActive; }

	FGameplayTag GetActionTag() const { return ActionTag; }

protected:
	// 실행 진입 훅. 기본 구현은 몽타주 재생 — 실패 시 false
	virtual bool OnActivate();

	// 종료 훅 (성공/실패/중단 공통)
	virtual void OnEnd(bool bSucceeded) {}

	// 종료 처리 + 컴포넌트에 통지. bStopMontage면 재생 중인 몽타주를 모든 머신에서 정지
	void FinishAction(bool bSucceeded, bool bStopMontage = false);

	// 몽타주 로드 → 모든 머신 재생 → 서버 종료 델리게이트 바인딩. 실패 시 false
	bool StartMontage();

	// 재생 중인 액션 몽타주가 끝났을 때 (서버). 기본: 정상 종료면 성공 / 끊기면 실패로 액션 종료
	virtual void OnMontageEnded(bool bInterrupted);

	// 서버 몽타주 종료 콜백
	void HandleMontageEnded(UAnimMontage* InMontage, bool bInterrupted);

	ACharacter* GetOwnerCharacter() const;

	AActor* GetTargetActor() const { return TargetActor.Get(); }

	// 적용할 데미지 (Damage가 0 이하이면 EnemyData의 AttackDamage)
	int32 GetDamage() const { return Damage > 0 ? Damage : DefaultDamage; }

	// 재생할 몽타주
	UPROPERTY(EditAnywhere, Category = "Action")
	TSoftObjectPtr<UAnimMontage> Montage;

	// 시작 섹션 (None이면 처음부터)
	UPROPERTY(EditAnywhere, Category = "Action")
	FName MontageSection = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Action", meta = (ClampMin = "0.01"))
	float PlayRate = 1.0f;

	// 종료 후 재사용 대기 시간(초)
	UPROPERTY(EditAnywhere, Category = "Action", meta = (ClampMin = "0.0"))
	float Cooldown = 2.0f;

	// 사용 가능 사거리 (타깃과의 거리). MaxRange 0 이하면 무제한
	UPROPERTY(EditAnywhere, Category = "Action", meta = (ClampMin = "0.0"))
	float MinRange = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Action", meta = (ClampMin = "0.0"))
	float MaxRange = 200.0f;

	// 0 이하이면 EnemyData의 AttackDamage 사용
	UPROPERTY(EditAnywhere, Category = "Action")
	int32 Damage = 0;

	// 판정 디버그 드로우 (컴포넌트 멀티캐스트로 전 머신에 표시)
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bDrawDebug", ClampMin = "0.0"))
	float DebugDrawDuration = 1.0f;

	// --- 런타임 (적마다 복제된 인스턴스에서만 유효) ---
	UPROPERTY(Transient)
	TObjectPtr<ULSAIActionComponent> OwnerComp;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> LoadedMontage;

	TWeakObjectPtr<AActor> TargetActor;

	FGameplayTag ActionTag;

	int32 DefaultDamage = 0;

	bool bIsActive = false;

	// 마지막 종료 시각 (쿨다운 기준)
	float LastEndTime = -UE_BIG_NUMBER;
};
