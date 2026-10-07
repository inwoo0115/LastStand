// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/LSAnimNotifyState_AIActionHit.h"
#include "AI/Components/LSAIActionComponent.h"
#include "Components/SkeletalMeshComponent.h"

void ULSAnimNotifyState_AIActionHit::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	SendHitWindow(MeshComp, true);
}

void ULSAnimNotifyState_AIActionHit::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	SendHitWindow(MeshComp, false);
}

FString ULSAnimNotifyState_AIActionHit::GetNotifyName_Implementation() const
{
	return WindowName.IsNone() ? TEXT("AI Hit Window") : FString::Printf(TEXT("AI Hit: %s"), *WindowName.ToString());
}

void ULSAnimNotifyState_AIActionHit::SendHitWindow(USkeletalMeshComponent* MeshComp, bool bBegin) const
{
	// 판정은 서버 권위 — 클라/에디터 프리뷰는 무시
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	if (ULSAIActionComponent* ActionComp = ULSAIActionComponent::FindActionComponent(Owner))
	{
		ActionComp->NotifyHitWindow(WindowName, bBegin);
	}
}
