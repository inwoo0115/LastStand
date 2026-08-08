// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSStatComponent.h"
#include "Net/UnrealNetwork.h"
#include "DataTable/LSDataSubsystem.h"
#include "DataTable/LSEnemyData.h"
#include "UI/LSUIEventSubsystem.h"
#include "GameFramework/Pawn.h"

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

	// 리슨 호스트가 플레이어인 경우 자신의 HUD 갱신 (데디 서버는 가드로 no-op)
	BroadcastHealthToUI();

	if (CurrentHealth <= 0)
	{
		OnDeath.Broadcast();
	}
}

void ULSStatComponent::CalculateDamage(int32 RawDamage)
{
	// TODO: 추후 방어력 등 반영한 계산식으로 확장
	const int32 FinalDamage = RawDamage;

	// 데미지 UI 이벤트 브로드캐스트 (호출한 머신=로컬 클라 기준)
	if (UGameInstance* GI = GetOwner()->GetGameInstance())
	{
		if (ULSUIEventSubsystem* UISub = GI->GetSubsystem<ULSUIEventSubsystem>())
		{
			UISub->DamageEvent.Broadcast(FinalDamage);
		}
	}
}

void ULSStatComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	// 클라 측: 최초 리플리케이션(초기화)과 이후 체력 변경을 플레이어 HUD로 전파
	BroadcastHealthToUI();

	if (CurrentHealth <= 0)
	{
		OnDeath.Broadcast();
	}
}

void ULSStatComponent::BroadcastHealthToUI()
{
	// 적(AI)·원격 플레이어·데디 서버 제외: 로컬 조종 + 플레이어 조종 폰만 (HUD 소유 클라)
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled() || !OwnerPawn->IsPlayerControlled())
	{
		return;
	}

	if (UGameInstance* GI = GetOwner()->GetGameInstance())
	{
		if (ULSUIEventSubsystem* UISub = GI->GetSubsystem<ULSUIEventSubsystem>())
		{
			UISub->HealthEvent.Broadcast(CurrentHealth, MaxHealth);
		}
	}
}

void ULSStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULSStatComponent, CurrentHealth);
	DOREPLIFETIME(ULSStatComponent, MaxHealth);
	DOREPLIFETIME(ULSStatComponent, AttackDamage);
}
