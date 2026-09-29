// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LSStatComponent.generated.h"

// 체력 변경 브로드캐스트 (NewCurrent, NewMax)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, int32, int32);

// 사망 브로드캐스트
DECLARE_MULTICAST_DELEGATE(FOnStatDeath);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTSTAND_API ULSStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULSStatComponent();

	void InitializeStatByEnemyData(FName EnemyName);

	// 데미지 적용 (서버 권위) — 체력 감소/클램프/사망 처리
	// DamageCauser: 데미지를 넣은 액터 (추후 어그로/타겟 선정용, null 허용)
	void ApplyDamage(int32 Damage, AActor* DamageCauser);

	// 받은 데미지를 계산(추후 방어력 등 반영)하고 데미지 UI 이벤트를 브로드캐스트
	void CalculateDamage(int32 RawDamage);

	int32 GetMaxHealth() const { return MaxHealth; }
	int32 GetCurrentHealth() const { return CurrentHealth; }
	int32 GetAttackDamage() const { return AttackDamage; }

	// 서버: 누적 데미지가 가장 큰 유효 주체 반환 (없거나 모두 파괴됐으면 nullptr)
	AActor* GetTopDamageCauser() const;

	// 체력 변경 시 브로드캐스트 (UI/AI 연동)
	FOnHealthChanged OnHealthChanged;

	// 사망 시 브로드캐스트 (사망 후속 처리 훅)
	FOnStatDeath OnDeath;

protected:
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentHealth();

	// 로컬 플레이어 소유 폰일 때만 UI HealthEvent로 현재/최대 체력 전파 (HUD 소유 클라)
	void BroadcastHealthToUI();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	int32 CurrentHealth = 100;

	UPROPERTY(Replicated)
	int32 MaxHealth = 100;

	UPROPERTY(Replicated)
	int32 AttackDamage = 0;

	// 서버: 데미지 주체별 누적 데미지 (주체 파괴 대비 약참조)
	TMap<TWeakObjectPtr<AActor>, int32> DamageCauserMap;
};
