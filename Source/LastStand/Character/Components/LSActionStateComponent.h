// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "LSActionStateComponent.generated.h"

// 액션 1개의 실행 규칙
USTRUCT(BlueprintType)
struct FLSActionRule
{
	GENERATED_BODY()

	// 이 상태 중 하나라도 활성이면 액션 거부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "State"))
	FGameplayTagContainer BlockedByStates;

	// 액션 실행 시 취소할 진행 중 상태 (우선순위가 더 낮은 행동)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "State"))
	FGameplayTagContainer CancelStates;
};

// 상태 태그 + 액션 규칙으로 입력 차단/취소 판정
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LASTSTAND_API ULSActionStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULSActionStateComponent();

	// 규칙의 BlockedByStates와 현재 상태가 겹치지 않으면 true (규칙 없는 액션은 허용)
	bool CanPerformAction(const FGameplayTag& Action) const;

	// 규칙의 CancelStates에 해당하는 진행 중 행동 취소
	void CancelStatesFor(const FGameplayTag& Action);

	// 현재 활성 상태 수집 (CMC/무기 조회 + LooseStates)
	FGameplayTagContainer GetActiveStates() const;

	// 출처가 따로 없는 상태용 (스턴 등)
	void AddLooseState(const FGameplayTag& State) { LooseStates.AddTag(State); }
	void RemoveLooseState(const FGameplayTag& State) { LooseStates.RemoveTag(State); }

protected:
	UPROPERTY(EditAnywhere, Category = "Action", meta = (Categories = "Action.Player"))
	TMap<FGameplayTag, FLSActionRule> ActionRules;

	FGameplayTagContainer LooseStates;

private:
	class ALSWeaponBase* GetFocusWeapon() const;
};
