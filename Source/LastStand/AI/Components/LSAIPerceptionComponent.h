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

	// 감지 볼륨 크기 (에디터 조정)
	UPROPERTY(EditAnywhere, Category = "Perception")
	float DetectionRadius = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Perception")
	float DetectionHalfHeight = 200.0f;

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
