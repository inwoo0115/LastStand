// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Components/LSAIPerceptionComponent.h"
#include "Character/LSCharacterBase.h"
#include "GameFramework/Actor.h"

ULSAIPerceptionComponent::ULSAIPerceptionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 감지 볼륨 크기 초기화
	InitCapsuleSize(DetectionRadius, DetectionHalfHeight);

	// 쿼리 전용 + Pawn만 오버랩 (물리 충돌 없음)
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetGenerateOverlapEvents(true);
}

void ULSAIPerceptionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 감지 계산은 서버 권위에서만. 클라에선 틱/오버랩 불필요
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		OnComponentBeginOverlap.AddUniqueDynamic(this, &ULSAIPerceptionComponent::OnCapsuleBeginOverlap);
		OnComponentEndOverlap.AddUniqueDynamic(this, &ULSAIPerceptionComponent::OnCapsuleEndOverlap);

		// 스폰 시점에 이미 겹쳐 있는 액터 초기 반영
		UpdateOverlaps();
	}
	else
	{
		SetComponentTickEnabled(false);
	}
}

void ULSAIPerceptionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 플레이어가 이동하므로 매 틱 최근접 재계산
	UpdateClosestTarget();
}

void ULSAIPerceptionComponent::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 플레이어(ALSCharacterBase 파생)만 후보로. 적 자신·다른 적은 여기서 걸러짐
	if (Cast<ALSCharacterBase>(OtherActor))
	{
		PerceivedActors.AddUnique(OtherActor);
	}
}

void ULSAIPerceptionComponent::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (PerceivedActors.Remove(OtherActor) > 0)
	{
		// 범위 이탈 즉시 타깃 재선정
		UpdateClosestTarget();
	}
}

void ULSAIPerceptionComponent::UpdateClosestTarget()
{
	const FVector Origin = GetComponentLocation();

	AActor* Closest = nullptr;
	float ClosestDistSq = TNumericLimits<float>::Max();

	// 역순 순회로 무효 포인터 정리하며 최소 거리 후보 선정
	for (int32 Index = PerceivedActors.Num() - 1; Index >= 0; --Index)
	{
		AActor* Candidate = PerceivedActors[Index];
		if (!IsValid(Candidate))
		{
			PerceivedActors.RemoveAt(Index);
			continue;
		}

		const float DistSq = FVector::DistSquared(Origin, Candidate->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			Closest = Candidate;
		}
	}

	TargetActor = Closest;
}
