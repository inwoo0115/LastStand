// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "DataTable/LSEnemyData.h"
#include "LSAIPerceptionComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LASTSTAND_API ULSAIPerceptionComponent : public UCapsuleComponent
{
	GENERATED_BODY()

public:
	ULSAIPerceptionComponent();

	// EnemyData에서 타겟 선정 방식/변경 딜레이 로드 (서버 전용)
	void InitializePerceptionByEnemyData(FName EnemyName);

	AActor* GetTargetActor() const { return TargetActor; }

	// 감지/타깃 선정 중지 (사망 시, 서버 전용) — 틱·오버랩 해제 + 타깃/후보 초기화
	void StopPerception();

	// 선정 방식에 따라 고른 현재 타깃 (서버 전용, 복제 안 함)
	UPROPERTY()
	TObjectPtr<AActor> TargetActor = nullptr;
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 딜레이/유효성 판단 후 필요할 때만 TargetActor 교체
	void UpdateTarget();

	// 방식별 후보 선정 (PerceivedActors 중에서)
	AActor* SelectClosestTarget() const;
	AActor* SelectTopDamageTarget() const;
	AActor* SelectRandomTarget() const;

	// 무감지 지속 판정 → bHasDetectedTarget 해제 + 캡슐 롤백 (서버 전용)
	void UpdateAggroReset();

	// 감지 볼륨 크기 (에디터 조정)
	UPROPERTY(EditAnywhere, Category = "Perception")
	float DetectionRadius = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Perception")
	float DetectionHalfHeight = 200.0f;

	// 첫 감지 이후 확장되는 어그로 유지 범위 (에디터 조정)
	UPROPERTY(EditAnywhere, Category = "Perception")
	float AggroRadius = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Perception")
	float AggroHalfHeight = 400.0f;

	// 감지 후보가 이 시간(초) 이상 비어 있으면 어그로 해제 + 캡슐 롤백
	UPROPERTY(EditAnywhere, Category = "Perception")
	float AggroResetDelay = 5.0f;

	// 롤백용 원래 캡슐 크기 (BeginPlay에서 캡처 — BP에서 캡슐 크기를 직접 바꿔도 반영)
	float DefaultRadius = 0.0f;
	float DefaultHalfHeight = 0.0f;

	// 후보가 비기 시작한 시각 (< 0 이면 비어있지 않음)
	float EmptyStartTime = -1.0f;

	// 캡슐 안의 ALSCharacterBase 후보들 (서버 전용)
	UPROPERTY()
	TArray<TObjectPtr<AActor>> PerceivedActors;

	// 타겟 선정 방식 (EnemyData에서 로드)
	ELSTargetSelectType TargetSelectType = ELSTargetSelectType::Closest;

	// 타겟 변경 딜레이(초) (EnemyData에서 로드)
	float TargetChangeDelay = 0.0f;

	// 마지막 타겟 교체 시각 (GetWorld()->GetTimeSeconds())
	float LastTargetChangeTime = 0.0f;
};
