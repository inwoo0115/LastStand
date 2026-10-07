// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/NetSerialization.h"
#include "LSAIActionComponent.generated.h"

class ULSAIActionBase;
class UAnimMontage;

// 행동 종료 (행동 태그, 성공 여부) — 서버
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAIActionFinished, FGameplayTag /*ActionTag*/, bool /*bSucceeded*/);

// 적 AI 행동 실행기 (서버 권위). 행동 선택은 StateTree가, 실행/쿨다운/연출 동기화는 이 컴포넌트가 담당
// 한 번에 하나의 행동만 실행. 몽타주는 멀티캐스트로 서버 포함 모든 머신에서 재생
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LASTSTAND_API ULSAIActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULSAIActionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// EnemyData의 Actions 클래스를 로드해 적 전용 인스턴스로 등록 (서버 전용)
	void InitializeActionsByEnemyData(FName EnemyName);

	// 쿨다운·사거리·실행 중 여부 검사
	bool CanRunAction(FGameplayTag ActionTag, AActor* Target) const;

	// 행동 실행. 실행 불가/시작 실패 시 false
	bool StartAction(FGameplayTag ActionTag, AActor* Target);

	// 실행 중인 행동 중단 (실패로 종료)
	void CancelCurrentAction();

	bool IsRunning() const { return CurrentAction != nullptr; }

	ULSAIActionBase* GetCurrentAction() const { return CurrentAction; }

	// 애님 노티파이 스테이트 → 현재 행동에 히트 판정 구간 전달 (서버)
	void NotifyHitWindow(FName WindowName, bool bBegin);

	// 행동이 호출: 모든 머신에서 몽타주 재생/정지
	void PlayActionMontage(UAnimMontage* InMontage, FName Section, float PlayRate);

	void StopActionMontage(UAnimMontage* InMontage);

	// 행동이 호출 (서버): 판정 디버그를 모든 머신에 그림
	void DrawDebugSweep(const FVector& Start, const FVector& End, float Radius, bool bHit, float Duration);

	void DrawDebugHitscan(const FVector& Start, const FVector& Direction, float Range, float HalfAngleDeg,
		const TArray<FVector_NetQuantize>& Ends, const TArray<bool>& Hits, float Duration);

	// 행동이 종료 시 호출
	void HandleActionFinished(ULSAIActionBase* Action, bool bSucceeded);

	// StateTree 소유자(AI 컨트롤러) 또는 폰에서 행동 컴포넌트 조회
	static ULSAIActionComponent* FindActionComponent(AActor* OwnerOrController);

	FOnAIActionFinished OnActionFinished;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 행동 시작/중단은 상태 전환 → Reliable
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayActionMontage(UAnimMontage* InMontage, FName Section, float PlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopActionMontage(UAnimMontage* InMontage);

	// 디버그 전용 → Unreliable
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDrawDebugSweep(FVector_NetQuantize Start, FVector_NetQuantize End, float Radius, bool bHit, float Duration);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDrawDebugHitscan(FVector_NetQuantize Start, FVector_NetQuantizeNormal Direction, float Range, float HalfAngleDeg,
		const TArray<FVector_NetQuantize>& Ends, const TArray<bool>& Hits, float Duration);

	class UAnimInstance* GetOwnerAnimInstance() const;

	// 행동 태그 → 적 전용 행동 인스턴스 (서버)
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<ULSAIActionBase>> Actions;

	// 실행 중인 행동 (서버)
	UPROPERTY(Transient)
	TObjectPtr<ULSAIActionBase> CurrentAction;

	// 정지 시 블렌드 아웃 시간
	UPROPERTY(EditAnywhere, Category = "Action")
	float StopBlendOutTime = 0.2f;
};
