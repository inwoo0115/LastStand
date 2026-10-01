// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Components/LSAIEventHubComponent.h"
#include "AI/LSAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "GameFramework/Pawn.h"
#include "Tags/LSGameplayTags.h"

ULSAIEventHubComponent::ULSAIEventHubComponent()
{
	// 이벤트 전달만 담당 → 틱 불필요
	PrimaryComponentTick.bCanEverTick = false;
}

void ULSAIEventHubComponent::SendEvent(FGameplayTag EventTag, FConstStructView Payload, FName Origin)
{
	// AI 컨트롤러(StateTree)는 서버에만 존재
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	// AI.Event 하위 태그만 허용
	if (!EventTag.IsValid() || !EventTag.MatchesTag(LSAITags::Event))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] SendEvent: invalid AI event tag '%s'"), *GetNameSafe(GetOwner()), *EventTag.ToString());
		return;
	}

	UStateTreeComponent* StateTreeComp = ResolveStateTreeComponent();
	if (!StateTreeComp)
	{
		// possess 전이거나 BehaviorTree 폴백 중이면 무시
		UE_LOG(LogTemp, Verbose, TEXT("[%s] SendEvent: no StateTree component, '%s' dropped"), *GetNameSafe(GetOwner()), *EventTag.ToString());
		return;
	}

	StateTreeComp->SendStateTreeEvent(EventTag, Payload, Origin);
}

void ULSAIEventHubComponent::K2_SendEvent(FGameplayTag EventTag, const FInstancedStruct& Payload, FName Origin)
{
	SendEvent(EventTag, FConstStructView(Payload), Origin);
}

UStateTreeComponent* ULSAIEventHubComponent::ResolveStateTreeComponent()
{
	if (CachedStateTreeComp.IsValid())
	{
		return CachedStateTreeComp.Get();
	}

	// possess가 BeginPlay보다 늦을 수 있으므로 필요 시점에 해석
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (ALSAIController* AIController = Cast<ALSAIController>(OwnerPawn->GetController()))
		{
			CachedStateTreeComp = AIController->GetStateTreeComponent();
		}
	}

	return CachedStateTreeComp.Get();
}
