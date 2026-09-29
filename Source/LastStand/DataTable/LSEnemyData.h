// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LSEnemyData.generated.h"

// 적 AI 타겟 선정 방식
UENUM(BlueprintType)
enum class ELSTargetSelectType : uint8
{
    Closest    UMETA(DisplayName = "Closest"),
    TopDamage  UMETA(DisplayName = "Top Damage"),
    Random     UMETA(DisplayName = "Random"),
    RandomLocked UMETA(DisplayName = "Random Locked"),   // 랜덤 선정 후 타겟 상실 시에만 재선정
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
};