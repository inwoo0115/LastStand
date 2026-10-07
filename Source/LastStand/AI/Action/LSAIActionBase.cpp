// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Action/LSAIActionBase.h"
#include "AI/Components/LSAIActionComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

UWorld* ULSAIActionBase::GetWorld() const
{
	// CDO(BP 기본값 편집 등)는 월드 없음 → 런타임 인스턴스만 컴포넌트 월드 사용
	return OwnerComp ? OwnerComp->GetWorld() : nullptr;
}

void ULSAIActionBase::InitializeAction(ULSAIActionComponent* InOwnerComp, FGameplayTag InActionTag, int32 InDefaultDamage)
{
	OwnerComp = InOwnerComp;
	ActionTag = InActionTag;
	DefaultDamage = InDefaultDamage;
}

bool ULSAIActionBase::CanActivate(AActor* InTarget) const
{
	const UWorld* World = GetWorld();
	const AActor* Owner = OwnerComp ? OwnerComp->GetOwner() : nullptr;
	if (bIsActive || !World || !Owner)
	{
		return false;
	}

	// 쿨다운
	if (World->GetTimeSeconds() - LastEndTime < Cooldown)
	{
		return false;
	}

	// 사거리 (타깃 없으면 사거리 검사 생략)
	if (InTarget)
	{
		const float Distance = FVector::Dist(Owner->GetActorLocation(), InTarget->GetActorLocation());
		if (Distance < MinRange || (MaxRange > 0.0f && Distance > MaxRange))
		{
			return false;
		}
	}

	return true;
}

bool ULSAIActionBase::Activate(AActor* InTarget)
{
	if (bIsActive || !OwnerComp)
	{
		return false;
	}

	TargetActor = InTarget;
	bIsActive = true;

	if (!OnActivate())
	{
		bIsActive = false;
		TargetActor = nullptr;
		return false;
	}

	return true;
}

void ULSAIActionBase::Cancel()
{
	FinishAction(false, true);
}

bool ULSAIActionBase::OnActivate()
{
	return StartMontage();
}

bool ULSAIActionBase::StartMontage()
{
	ACharacter* OwnerCh = GetOwnerCharacter();
	if (!OwnerCh || Montage.IsNull())
	{
		return false;
	}

	LoadedMontage = Montage.LoadSynchronous();
	if (!LoadedMontage)
	{
		return false;
	}

	// 서버 포함 모든 머신 재생 (서버 재생 → 노티파이/판정 타이밍 확보)
	OwnerComp->PlayActionMontage(LoadedMontage, MontageSection, PlayRate);

	UAnimInstance* AnimInstance = OwnerCh->GetMesh() ? OwnerCh->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance || !AnimInstance->Montage_IsPlaying(LoadedMontage))
	{
		return false;
	}

	// 서버 몽타주 종료 → OnMontageEnded
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ULSAIActionBase::HandleMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, LoadedMontage);

	return true;
}

void ULSAIActionBase::FinishAction(bool bSucceeded, bool bStopMontage)
{
	if (!bIsActive)
	{
		return;
	}

	// 몽타주 정지 시 발생하는 종료 콜백 재진입 방지를 위해 먼저 비활성화
	bIsActive = false;

	if (const UWorld* World = GetWorld())
	{
		LastEndTime = World->GetTimeSeconds();
	}

	if (bStopMontage && LoadedMontage && OwnerComp)
	{
		OwnerComp->StopActionMontage(LoadedMontage);
	}

	OnEnd(bSucceeded);

	TargetActor = nullptr;

	if (OwnerComp)
	{
		OwnerComp->HandleActionFinished(this, bSucceeded);
	}
}

void ULSAIActionBase::HandleMontageEnded(UAnimMontage* InMontage, bool bInterrupted)
{
	if (!bIsActive || InMontage != LoadedMontage)
	{
		return;
	}

	OnMontageEnded(bInterrupted);
}

void ULSAIActionBase::OnMontageEnded(bool bInterrupted)
{
	// 다른 몽타주(피격 등)에 끊기면 실패
	FinishAction(!bInterrupted);
}

ACharacter* ULSAIActionBase::GetOwnerCharacter() const
{
	return OwnerComp ? Cast<ACharacter>(OwnerComp->GetOwner()) : nullptr;
}
