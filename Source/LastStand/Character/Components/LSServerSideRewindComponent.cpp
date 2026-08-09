// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSServerSideRewindComponent.h"
#include "Character/Components/LSHitboxComponent.h"
#include "Interface/LSHitboxInterface.h"

namespace
{
	// 세그먼트 vs 방향성 박스(OBB) 교차 테스트.
	// 세그먼트를 박스 로컬 공간으로 옮겨 원점 정렬 AABB와 교차 검사한다.
	static bool SegmentIntersectsBox(const FHitBoxSnapshot& Box, const FVector& Start, const FVector& End)
	{
		const FTransform BoxTM(Box.Rotation, Box.Location);   // 스케일 없음(Extent에 이미 반영)
		const FVector LocalStart = BoxTM.InverseTransformPosition(Start);
		const FVector LocalEnd   = BoxTM.InverseTransformPosition(End);
		const FBox LocalBox(-Box.BoxExtent, Box.BoxExtent);
		return FMath::LineBoxIntersection(LocalBox, LocalStart, LocalEnd, LocalEnd - LocalStart);
	}
}


// Sets default values for this component's properties
ULSServerSideRewindComponent::ULSServerSideRewindComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void ULSServerSideRewindComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();

	// 히스토리는 서버에서만 의미가 있으므로 클라에서는 기록 안 함
	if (!Owner || !Owner->HasAuthority())
	{
		SetComponentTickEnabled(false);
		return;
	}

	// 소유 pawn의 전체 히트박스를 인터페이스로 수집 (개수 가변)
	if (Owner->Implements<ULSHitboxInterface>())
	{
		TArray<ULSHitboxComponent*> Found;
		Cast<ILSHitboxInterface>(Owner)->GetHitboxComponents(Found);

		CachedHitboxes.Reset();
		for (ULSHitboxComponent* Hitbox : Found)
		{
			CachedHitboxes.Add(Hitbox);
		}
	}
}


// Called every frame
void ULSServerSideRewindComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UWorld* World = GetWorld();
	if (!World || CachedHitboxes.Num() == 0)
	{
		return;
	}

	// 서버 게임시간 기준 20ms 간격 게이트
	const float Now = World->GetTimeSeconds();
	if (Now - LastRecordTime < RecordInterval)
	{
		return;
	}
	LastRecordTime = Now;

	// 현재 시점의 전체 히트박스 스냅샷 기록
	FServerSideRewindSnapshot Snapshot;
	Snapshot.Time = Now;
	for (ULSHitboxComponent* Hitbox : CachedHitboxes)
	{
		if (!Hitbox)
		{
			continue;
		}

		FHitBoxSnapshot Box;
		Box.Location = Hitbox->GetComponentLocation();
		Box.Rotation = Hitbox->GetComponentRotation();
		Box.BoxExtent = Hitbox->GetScaledBoxExtent();

		// FName = 컴포넌트 인스턴스 이름 (리와인드 시 라이브 컴포넌트와 매칭할 안정적 키)
		Snapshot.HitBoxSnapshots.Add(Hitbox->GetFName(), Box);
	}
	History.Add(MoveTemp(Snapshot));

	// End(200ms)보다 오래된 스냅샷을 front에서 제거 (큐 FIFO 만료)
	while (History.Num() > 0 && (Now - History[0].Time) > HistoryEndOffset)
	{
		History.RemoveAt(0);
	}
}


bool ULSServerSideRewindComponent::ConfirmHit(const FVector& TraceStart, const FVector& TraceEnd, float Timestamp) const
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();

	// 판정은 서버에서만
	if (!Owner || !Owner->HasAuthority() || !World)
	{
		return false;
	}
	if (History.Num() == 0)
	{
		return false;
	}

	const float Now = World->GetTimeSeconds();

	// 리와인드 유효 구간 [Now-200ms, Now]로 쿼리 시간 클램프
	float QueryTime = Timestamp;
	if (Now - QueryTime > HistoryEndOffset)
	{
		QueryTime = Now - HistoryEndOffset;   // 200ms 이상 차이 → 200ms로 비교
	}
	QueryTime = FMath::Min(QueryTime, Now);   // 미래 타임스탬프 방어

	// 가장 가까운 단일 스냅샷 선택 (|Time - QueryTime| 최소)
	const FServerSideRewindSnapshot* Best = nullptr;
	float BestDelta = TNumericLimits<float>::Max();
	for (const FServerSideRewindSnapshot& Snap : History)
	{
		const float Delta = FMath::Abs(Snap.Time - QueryTime);
		if (Delta < BestDelta)
		{
			BestDelta = Delta;
			Best = &Snap;
		}
	}
	if (!Best)
	{
		return false;
	}

	// 스냅샷 내 모든 히트박스 중 하나라도 세그먼트와 교차하면 명중
	for (const TPair<FName, FHitBoxSnapshot>& Pair : Best->HitBoxSnapshots)
	{
		if (SegmentIntersectsBox(Pair.Value, TraceStart, TraceEnd))
		{
			return true;
		}
	}
	return false;
}

