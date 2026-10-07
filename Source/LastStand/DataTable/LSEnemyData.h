// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "LSEnemyData.generated.h"

class UStateTree;
class ULSAIActionBase;

// 적 AI 타겟 선정 방식
UENUM(BlueprintType)
enum class ELSTargetSelectType : uint8
{
    Closest    UMETA(DisplayName = "Closest"),
    TopDamage  UMETA(DisplayName = "Top Damage"),
    Random     UMETA(DisplayName = "Random"),
    RandomLocked UMETA(DisplayName = "Random Locked"),   // 랜덤 선정 후 타겟 상실 시에만 재선정
};

// 체력 비율 임계치 → StateTree 이벤트 (임계치마다 1회 발동)
USTRUCT(BlueprintType)
struct FLSHealthThresholdEvent
{
    GENERATED_BODY()

    // 현재/최대 체력 비율이 이 값 이하가 되면 발동 (0~1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HealthRatio = 0.5f;

    // 발동 시 전환될 페이즈 번호 (ALSEnemyBase::CurrentPhase에 기록, 이벤트 페이로드로 전달)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (ClampMin = "1"))
    int32 Phase = 1;

    // 전송할 이벤트 (비어 있으면 AI.Event.PhaseChange)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (Categories = "AI.Event"))
    FGameplayTag EventTag;
};

// 페이즈 전환 이벤트 페이로드 (StateTree 이벤트 전이의 Payload Struct로 지정해 분기/바인딩)
USTRUCT(BlueprintType)
struct FLSPhaseChangePayload
{
    GENERATED_BODY()

    // 전환된 페이즈 번호
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    int32 Phase = 0;
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    FName EnemyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 MaxHealth = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 CurrentHealth = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 AttackDamage = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    TSoftClassPtr<APawn> EnemyClass;

    // 타겟 선정 방식 (가장 가까운 대상 / 가장 많이 때린 대상 / 랜덤 / 랜덤 후 고정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Target")
    ELSTargetSelectType TargetSelectType = ELSTargetSelectType::Closest;

    // 타겟 변경 딜레이(초): 이 시간 동안은 현재 타겟 유지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Target")
    float TargetChangeDelay = 2.0f;

    // 서브트리 슬롯(AI.Subtree.*) → 교체할 Linked StateTree. 비어 있는 슬롯은 Base 트리의 기본 에셋 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI", meta = (ForceInlineRow, Categories = "AI.Subtree"))
    TMap<FGameplayTag, TSoftObjectPtr<UStateTree>> SubtreeOverrides;

    // 행동 태그(AI.Action.*) → 액션 클래스 (파라미터는 BP 서브클래스 기본값). 런타임에 적마다 인스턴스 생성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI", meta = (ForceInlineRow, Categories = "AI.Action"))
    TMap<FGameplayTag, TSoftClassPtr<ULSAIActionBase>> Actions;

    // 체력 비율 임계치별 페이즈 전환 이벤트 (각 1회, 서버에서 StateTree로 전송)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
    TArray<FLSHealthThresholdEvent> HealthThresholdEvents;
};