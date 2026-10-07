// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Components/LSAIActionComponent.h"
#include "AI/Action/LSAIActionBase.h"
#include "DataTable/LSDataSubsystem.h"
#include "Interface/LSAIActionComponentInterface.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"

ULSAIActionComponent::ULSAIActionComponent()
{
	// 행동 실행 중에만 틱 (StartAction에서 활성화)
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// 몽타주 멀티캐스트용
	SetIsReplicatedByDefault(true);
}

void ULSAIActionComponent::InitializeActionsByEnemyData(FName EnemyName)
{
	// 행동 실행은 서버 전용
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

	Actions.Reset();
	for (const TPair<FGameplayTag, TSoftClassPtr<ULSAIActionBase>>& Pair : Data->Actions)
	{
		if (!Pair.Key.IsValid() || Pair.Value.IsNull())
		{
			continue;
		}

		UClass* ActionClass = Pair.Value.LoadSynchronous();
		if (!ActionClass || ActionClass->HasAnyClassFlags(CLASS_Abstract))
		{
			continue;
		}

		// 적마다 인스턴스 생성 → 런타임 상태(쿨다운 등) 분리. 파라미터는 클래스 기본값(CDO)
		ULSAIActionBase* Action = NewObject<ULSAIActionBase>(this, ActionClass);
		Action->InitializeAction(this, Pair.Key, Data->AttackDamage);
		Actions.Add(Pair.Key, Action);
	}
}

bool ULSAIActionComponent::CanRunAction(FGameplayTag ActionTag, AActor* Target) const
{
	if (CurrentAction)
	{
		return false;
	}

	const TObjectPtr<ULSAIActionBase>* Action = Actions.Find(ActionTag);
	return Action && *Action && (*Action)->CanActivate(Target);
}

bool ULSAIActionComponent::StartAction(FGameplayTag ActionTag, AActor* Target)
{
	if (!CanRunAction(ActionTag, Target))
	{
		return false;
	}

	ULSAIActionBase* Action = Actions.FindRef(ActionTag);
	CurrentAction = Action;

	if (!Action->Activate(Target))
	{
		CurrentAction = nullptr;
		return false;
	}

	// 시작 도중 즉시 종료되지 않았으면 틱 활성화
	if (CurrentAction == Action)
	{
		SetComponentTickEnabled(true);
	}

	return true;
}

void ULSAIActionComponent::CancelCurrentAction()
{
	if (CurrentAction)
	{
		CurrentAction->Cancel();
	}
}

void ULSAIActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentAction)
	{
		CurrentAction->TickAction(DeltaTime);
	}
	else
	{
		SetComponentTickEnabled(false);
	}
}

void ULSAIActionComponent::NotifyHitWindow(FName WindowName, bool bBegin)
{
	if (CurrentAction)
	{
		CurrentAction->OnHitWindow(WindowName, bBegin);
	}
}

void ULSAIActionComponent::PlayActionMontage(UAnimMontage* InMontage, FName Section, float PlayRate)
{
	if (GetOwner()->HasAuthority() && InMontage)
	{
		MulticastPlayActionMontage(InMontage, Section, PlayRate);
	}
}

void ULSAIActionComponent::StopActionMontage(UAnimMontage* InMontage)
{
	if (GetOwner()->HasAuthority() && InMontage)
	{
		MulticastStopActionMontage(InMontage);
	}
}

void ULSAIActionComponent::DrawDebugSweep(const FVector& Start, const FVector& End, float Radius, bool bHit, float Duration)
{
	if (GetOwner()->HasAuthority())
	{
		MulticastDrawDebugSweep(Start, End, Radius, bHit, Duration);
	}
}

void ULSAIActionComponent::DrawDebugHitscan(const FVector& Start, const FVector& Direction, float Range, float HalfAngleDeg,
	const TArray<FVector_NetQuantize>& Ends, const TArray<bool>& Hits, float Duration)
{
	if (GetOwner()->HasAuthority())
	{
		MulticastDrawDebugHitscan(Start, Direction, Range, HalfAngleDeg, Ends, Hits, Duration);
	}
}

void ULSAIActionComponent::HandleActionFinished(ULSAIActionBase* Action, bool bSucceeded)
{
	if (!Action || Action != CurrentAction)
	{
		return;
	}

	CurrentAction = nullptr;
	SetComponentTickEnabled(false);

	OnActionFinished.Broadcast(Action->GetActionTag(), bSucceeded);
}

ULSAIActionComponent* ULSAIActionComponent::FindActionComponent(AActor* OwnerOrController)
{
	// StateTree 소유자는 AI 컨트롤러 → 폰으로 해석
	AActor* Actor = OwnerOrController;
	if (const AController* Controller = Cast<AController>(OwnerOrController))
	{
		Actor = Controller->GetPawn();
	}

	if (Actor && Actor->Implements<ULSAIActionComponentInterface>())
	{
		return Cast<ILSAIActionComponentInterface>(Actor)->GetAIActionComponent();
	}

	return nullptr;
}

void ULSAIActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 파괴 시 실행 중 행동 정리 (델리게이트 구독자에게 실패 통지)
	CancelCurrentAction();

	Super::EndPlay(EndPlayReason);
}

void ULSAIActionComponent::MulticastPlayActionMontage_Implementation(UAnimMontage* InMontage, FName Section, float PlayRate)
{
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (!AnimInstance || !InMontage)
	{
		return;
	}

	AnimInstance->Montage_Play(InMontage, PlayRate);
	if (Section != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(Section, InMontage);
	}
}

void ULSAIActionComponent::MulticastStopActionMontage_Implementation(UAnimMontage* InMontage)
{
	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		AnimInstance->Montage_Stop(StopBlendOutTime, InMontage);
	}
}

void ULSAIActionComponent::MulticastDrawDebugSweep_Implementation(FVector_NetQuantize Start, FVector_NetQuantize End, float Radius, bool bHit, float Duration)
{
#if ENABLE_DRAW_DEBUG
	// 데디 서버는 뷰포트 없음 → 스킵 (리슨 서버 호스트는 그림)
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// 스윕 궤적을 구체 스윕 체적(캡슐)으로 표시
	const FVector Delta = End - Start;
	const FVector Center = Start + Delta * 0.5f;
	const float HalfHeight = Delta.Size() * 0.5f + Radius;
	const FQuat Rotation = Delta.IsNearlyZero() ? FQuat::Identity : FRotationMatrix::MakeFromZ(Delta).ToQuat();

	DrawDebugCapsule(GetWorld(), Center, HalfHeight, Radius, Rotation, bHit ? FColor::Red : FColor::Green, false, Duration);
#endif
}

void ULSAIActionComponent::MulticastDrawDebugHitscan_Implementation(FVector_NetQuantize Start, FVector_NetQuantizeNormal Direction, float Range, float HalfAngleDeg,
	const TArray<FVector_NetQuantize>& Ends, const TArray<bool>& Hits, float Duration)
{
#if ENABLE_DRAW_DEBUG
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	UWorld* World = GetWorld();

	// 탄퍼짐 원뿔 범위
	const float HalfAngleRad = FMath::DegreesToRadians(HalfAngleDeg);
	DrawDebugCone(World, Start, Direction, Range, HalfAngleRad, HalfAngleRad, 16, FColor::Yellow, false, Duration);

	// 펠릿별 탄도 (명중 빨강 / 빗나감 초록) + 명중점
	for (int32 Index = 0; Index < Ends.Num(); ++Index)
	{
		const bool bHit = Hits.IsValidIndex(Index) && Hits[Index];
		DrawDebugLine(World, Start, Ends[Index], bHit ? FColor::Red : FColor::Green, false, Duration);
		if (bHit)
		{
			DrawDebugPoint(World, Ends[Index], 10.0f, FColor::Red, false, Duration);
		}
	}
#endif
}

UAnimInstance* ULSAIActionComponent::GetOwnerAnimInstance() const
{
	const ACharacter* OwnerCh = Cast<ACharacter>(GetOwner());
	return OwnerCh && OwnerCh->GetMesh() ? OwnerCh->GetMesh()->GetAnimInstance() : nullptr;
}
