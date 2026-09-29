// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Components/LSAIPerceptionComponent.h"
#include "Character/LSCharacterBase.h"
#include "Character/Components/LSStatComponent.h"
#include "Interface/LSStatComponentInterface.h"
#include "DataTable/LSDataSubsystem.h"
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

void ULSAIPerceptionComponent::InitializePerceptionByEnemyData(FName EnemyName)
{
	// 서버 권위에서만 초기화 (타겟 선정은 서버 전용)
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

	TargetSelectType = Data->TargetSelectType;
	TargetChangeDelay = Data->TargetChangeDelay;
}

void ULSAIPerceptionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 플레이어가 이동하므로 매 틱 타겟 재평가 (딜레이 내에선 유지)
	UpdateTarget();
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
		UpdateTarget();
	}
}

void ULSAIPerceptionComponent::UpdateTarget()
{
	// 역순 순회로 무효 포인터 정리
	for (int32 Index = PerceivedActors.Num() - 1; Index >= 0; --Index)
	{
		if (!IsValid(PerceivedActors[Index]))
		{
			PerceivedActors.RemoveAt(Index);
		}
	}

	const float Now = GetWorld()->GetTimeSeconds();

	// 현재 타겟이 파괴/범위 이탈이면 딜레이 무시하고 즉시 재선정
	const bool bTargetLost = !IsValid(TargetActor) || !PerceivedActors.Contains(TargetActor);

	// 랜덤 후 고정: 타겟을 잃기 전(파괴/범위 이탈)까지 딜레이와 무관하게 유지
	if (TargetSelectType == ELSTargetSelectType::RandomLocked && !bTargetLost)
	{
		return;
	}

	if (!bTargetLost && Now - LastTargetChangeTime < TargetChangeDelay)
	{
		return;
	}

	AActor* NewTarget = nullptr;
	switch (TargetSelectType)
	{
	case ELSTargetSelectType::Closest:
		NewTarget = SelectClosestTarget();
		break;
	case ELSTargetSelectType::TopDamage:
		NewTarget = SelectTopDamageTarget();
		break;
	case ELSTargetSelectType::Random:
		NewTarget = SelectRandomTarget();
		// 랜덤은 딜레이마다 한 번만 굴림 (변경 여부와 무관하게 타임스탬프 갱신)
		LastTargetChangeTime = Now;
		break;
	case ELSTargetSelectType::RandomLocked:
		NewTarget = SelectRandomTarget();
		break;
	}

	// 변경이 있을 때만 교체
	if (NewTarget != TargetActor)
	{
		TargetActor = NewTarget;
		LastTargetChangeTime = Now;
	}
}

AActor* ULSAIPerceptionComponent::SelectClosestTarget() const
{
	const FVector Origin = GetComponentLocation();

	AActor* Closest = nullptr;
	float ClosestDistSq = TNumericLimits<float>::Max();

	for (AActor* Candidate : PerceivedActors)
	{
		const float DistSq = FVector::DistSquared(Origin, Candidate->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			Closest = Candidate;
		}
	}

	return Closest;
}

AActor* ULSAIPerceptionComponent::SelectTopDamageTarget() const
{
	// owner StatComponent의 최다 데미지 주체가 감지 범위 안에 있으면 선택
	AActor* Owner = GetOwner();
	if (Owner && Owner->Implements<ULSStatComponentInterface>())
	{
		if (ULSStatComponent* Stat = Cast<ILSStatComponentInterface>(Owner)->GetStatComponent())
		{
			AActor* TopCauser = Stat->GetTopDamageCauser();
			if (TopCauser && PerceivedActors.Contains(TopCauser))
			{
				return TopCauser;
			}
		}
	}

	// 아직 피격 없음 / 범위 밖이면 최근접으로 대체
	return SelectClosestTarget();
}

AActor* ULSAIPerceptionComponent::SelectRandomTarget() const
{
	if (PerceivedActors.IsEmpty())
	{
		return nullptr;
	}

	return PerceivedActors[FMath::RandRange(0, PerceivedActors.Num() - 1)];
}
