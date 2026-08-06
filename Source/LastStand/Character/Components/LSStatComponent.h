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
	void ApplyDamage(int32 Damage);

	// 받은 데미지를 계산(추후 방어력 등 반영)하고 데미지 UI 이벤트를 브로드캐스트
	void CalculateDamage(int32 RawDamage);

	int32 GetMaxHealth() const { return MaxHealth; }
	int32 GetCurrentHealth() const { return CurrentHealth; }
	int32 GetAttackDamage() const { return AttackDamage; }

	// 체력 변경 시 브로드캐스트 (UI/AI 연동)
	FOnHealthChanged OnHealthChanged;

	// 사망 시 브로드캐스트 (사망 후속 처리 훅)
	FOnStatDeath OnDeath;

protected:
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentHealth();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	int32 CurrentHealth = 0;

	UPROPERTY(Replicated)
	int32 MaxHealth = 0;

	UPROPERTY(Replicated)
	int32 AttackDamage = 0;
};
