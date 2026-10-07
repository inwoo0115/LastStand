// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Action/LSAIAction_PlayMontage.h"
#include "AI/Components/LSAIActionComponent.h"
#include "AI/Components/LSAIEventHubComponent.h"
#include "Interface/LSAIEventHubInterface.h"

ULSAIAction_PlayMontage::ULSAIAction_PlayMontage()
{
	// 연출용 기본값: 사거리 무제한, 쿨다운 없음 (BP에서 변경 가능)
	MaxRange = 0.0f;
	Cooldown = 0.0f;
}

void ULSAIAction_PlayMontage::OnEnd(bool bSucceeded)
{
	Super::OnEnd(bSucceeded);

	if (!bSucceeded || !EventOnFinished.IsValid() || !OwnerComp)
	{
		return;
	}

	AActor* Owner = OwnerComp->GetOwner();
	if (Owner && Owner->Implements<ULSAIEventHubInterface>())
	{
		if (ULSAIEventHubComponent* EventHub = Cast<ILSAIEventHubInterface>(Owner)->GetAIEventHubComponent())
		{
			EventHub->SendEvent(EventOnFinished);
		}
	}
}
