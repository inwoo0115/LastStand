// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSStatComponent.h"
#include "Net/UnrealNetwork.h"
#include "DataTable/LSDataSubsystem.h"
#include "DataTable/LSEnemyData.h"

ULSStatComponent::ULSStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void ULSStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULSStatComponent::InitializeStatByEnemyData(FName EnemyName)
{
	// 서버 권위에서만 초기화 (리플리케이션으로 클라 반영)
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	ULSDataSubsystem* Sub = GetOwner()->GetGameInstance()->GetSubsystem<ULSDataSubsystem>();
	if (!Sub)
	{
		return;
	}

	const FEnemyData* Data = Sub->FindEnemy(EnemyName);
	if (!Data)
	{
		return;
	}

	MaxHealth = Data->MaxHealth;
	CurrentHealth = Data->CurrentHealth;   // 데이터 테이블의 초기 현재 체력 사용
	AttackDamage = Data->AttackDamage;

	// 서버 로컬 브로드캐스트(클라는 OnRep에서)
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void ULSStatComponent::ApplyDamage(int32 Damage)
{
	// 서버 권위에서만 체력 변경 (리플리케이션으로 클라 반영)
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// 유효하지 않은 데미지/이미 사망 상태면 무시
	if (Damage <= 0 || CurrentHealth <= 0)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0, MaxHealth);

	// 서버 로컬 브로드캐스트(클라는 OnRep_CurrentHealth에서)
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0)
	{
		OnDeath.Broadcast();
	}
}

void ULSStatComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0)
	{
		OnDeath.Broadcast();
	}
}

void ULSStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULSStatComponent, CurrentHealth);
	DOREPLIFETIME(ULSStatComponent, MaxHealth);
	DOREPLIFETIME(ULSStatComponent, AttackDamage);
}
