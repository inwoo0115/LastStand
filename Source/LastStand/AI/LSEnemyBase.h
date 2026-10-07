// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/LSStatComponentInterface.h"
#include "Interface/LSHitboxInterface.h"
#include "Interface/LSAIEventHubInterface.h"
#include "Interface/LSAIActionComponentInterface.h"
#include "GameplayTagContainer.h"
#include "DataTable/LSEnemyData.h"
#include "LSEnemyBase.generated.h"

UCLASS()
class LASTSTAND_API ALSEnemyBase : public ACharacter, public ILSStatComponentInterface, public ILSHitboxInterface, public ILSAIEventHubInterface, public ILSAIActionComponentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ALSEnemyBase();

	virtual class ULSStatComponent* GetStatComponent() override;

	virtual void ApplyDamage(int32 Damage, AActor* DamageCauser) override;

	// 이 적이 보유한 모든 부위 히트박스를 반환
	virtual void GetHitboxComponents(TArray<class ULSHitboxComponent*>& OutHitboxes) const override;

	// StateTree 이벤트 전달 허브 반환
	virtual class ULSAIEventHubComponent* GetAIEventHubComponent() override;

	// 행동(공격 등) 실행기 반환
	virtual class ULSAIActionComponent* GetAIActionComponent() override;

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FRotator GetCurrentControllerRotation() const { return CurrentControllerRotation; }

	bool GetTurnLeft() const { return bTurnLeft; }

	bool GetTurnRight() const { return bTurnRight; }

	bool GetIsRangeAttacking() const { return bIsRangeAttacking; }

	// 원거리 공격 중 플래그 세팅 (서버 전용 — 클라는 복제로 수신)
	void SetIsRangeAttacking(bool bInIsRangeAttacking);

	bool GetHasDetectedTarget() const { return bHasDetectedTarget; }

	// 생애 최초 감지 플래그 세팅 (서버 전용 — 클라는 복제로 수신)
	void SetHasDetectedTarget(bool bInHasDetectedTarget);

	int32 GetCurrentPhase() const { return CurrentPhase; }

	bool GetIsDead() const { return bIsDead; }

	class UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

	class UStateTree* GetStateTree() const { return StateTree; }

	class ULSAIPerceptionComponent* GetPerceptionComponent() const { return Perception; }

	FName GetEnemyName() const { return EnemyName; }

protected:
	virtual void BeginPlay() override;

	// EnemyData의 체력 임계치 이벤트 로드 + 체력 변경 구독 (서버 전용)
	void InitializePhaseByEnemyData();

	// 체력 변경 → 임계치 통과 시 페이즈 갱신 + StateTree 이벤트 전송 (서버)
	void HandleHealthChanged(int32 NewCurrentHealth, int32 NewMaxHealth);

	// 체력 0 → 사망 플래그 + StateTree에 Death 이벤트 (서버, 1회)
	void HandleDeath();

	// 스탯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSStatComponent> Stat;

	// 서버 사이드 리와인드(히트박스 히스토리 기록) 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSServerSideRewindComponent> ServerSideRewind;

	// AI 퍼셉션(가장 가까운 플레이어 탐지) 컴포넌트 — 서버 전용 계산
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSAIPerceptionComponent> Perception;

	// AI 이벤트 허브(게임플레이 → StateTree 이벤트 전달 통로) — 서버 전용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSAIEventHubComponent> EventHub;

	// AI 행동 실행기(StateTree Task가 태그로 실행) — 서버 실행, 몽타주만 멀티캐스트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSAIActionComponent> ActionComp;

	// 스탯 초기화에 사용할 데이터 테이블(FEnemyData) 행 이름
	UPROPERTY(EditAnywhere, Category = Stat, meta = (AllowPrivateAccess = "true"))
	FName EnemyName;

	// 체력바 위젯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UWidgetComponent> HealthBarWidget;

	// --- 부위별 히트박스 (본 소켓 재부착·크기 조정은 BP에서 마무리) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> HeadHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> TorsoHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> LeftArmHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> RightArmHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> LeftLegHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> RightLegHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> WeakPointHitbox;

	// 비헤이비어 트리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBehaviorTree> BehaviorTree;

	// 스테이트 트리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStateTree> StateTree;

	// State Tree에서 상태 분기에 사용하는 현재 AI 상태 태그 (서버 전용, 비복제)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer StateTags;

	// AI 컨트롤러(서버 전용)의 컨트롤 회전 — 클라 애님 블루프린트용 복제
	UPROPERTY(Replicated)
	FRotator CurrentControllerRotation = FRotator::ZeroRotator;

	// 액터(몸통)가 컨트롤 회전을 따라가는 보간 속도
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float TurnInterpSpeed = 3.0f;

	// 턴 플래그 활성화 각도 차이(도)
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (AllowPrivateAccess = "true"))
	float TurnThresholdAngle = 60.0f;

	// 컨트롤 회전이 액터보다 왼쪽/오른쪽으로 TurnThresholdAngle 이상 차이 (서버 계산, 클라 복제)
	UPROPERTY(Replicated)
	bool bTurnLeft = false;

	UPROPERTY(Replicated)
	bool bTurnRight = false;

	// 원거리 공격(Hitscan 액션) 실행 중 (서버 세팅, 클라 복제 — 애님용)
	UPROPERTY(Replicated)
	bool bIsRangeAttacking = false;

	// 생애 최초 타깃 감지 여부 (서버 세팅, 클라 복제 — 애님용 / Initialization 이벤트 1회 게이트)
	UPROPERTY(Replicated)
	bool bHasDetectedTarget = false;

	// 현재 페이즈 (0 = 시작, 임계치 발동 시 해당 항목의 Phase) — 서버 세팅, 클라 복제
	UPROPERTY(Replicated)
	int32 CurrentPhase = 0;

	// 사망 여부 (서버 세팅, 클라 복제 — 애님용)
	UPROPERTY(Replicated)
	bool bIsDead = false;

	// 체력 비율 내림차순으로 정렬한 임계치 이벤트 (서버 전용)
	TArray<FLSHealthThresholdEvent> HealthThresholdEvents;

	// 다음에 검사할 임계치 인덱스 (이전 항목은 발동/소비됨)
	int32 NextHealthThresholdIndex = 0;
};
