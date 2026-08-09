// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSServerSideRewindComponent.h"
#include "Character/Components/LSHitboxComponent.h"
#include "Interface/LSHitboxInterface.h"
#include "DrawDebugHelpers.h"

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

	// 두 박스 스냅샷을 Alpha로 선형 보간 (회전은 Slerp)
	static FHitBoxSnapshot InterpolateBox(const FHitBoxSnapshot& A, const FHitBoxSnapshot& B, float Alpha)
	{
		FHitBoxSnapshot Out;
		Out.Location  = FMath::Lerp(A.Location, B.Location, Alpha);
		Out.Rotation  = FQuat::Slerp(FQuat(A.Rotation), FQuat(B.Rotation), Alpha).Rotator();
		Out.BoxExtent = FMath::Lerp(A.BoxExtent, B.BoxExtent, Alpha);
		return Out;
	}
}


// Sets default values for this component's properties
ULSServerSideRewindComponent::ULSServerSideRewindComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// NetMulticast RPC가 클라로 라우팅되도록 컴포넌트 리플리케이트 (프로퍼티 복제는 없음)
	SetIsReplicatedByDefault(true);
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

	// 디버그: 0.5초마다 현재 히트박스 위치를 멀티캐스트로 전 클라에 전송해 그림 (저장 주기와 별개)
	if (bDrawDebugSnapshot && Now - LastDebugDrawTime >= DebugDrawInterval)
	{
		LastDebugDrawTime = Now;

		TArray<FHitBoxSnapshot> DebugBoxes;
		DebugBoxes.Reserve(CachedHitboxes.Num());
		for (ULSHitboxComponent* Hitbox : CachedHitboxes)
		{
			if (!Hitbox)
			{
				continue;
			}

			FHitBoxSnapshot Box;
			Box.Location  = Hitbox->GetComponentLocation();
			Box.Rotation  = Hitbox->GetComponentRotation();
			Box.BoxExtent = Hitbox->GetScaledBoxExtent();
			DebugBoxes.Add(Box);
		}
		MulticastDrawDebugBoxes(DebugBoxes);
	}
}


void ULSServerSideRewindComponent::MulticastDrawDebugBoxes_Implementation(const TArray<FHitBoxSnapshot>& Boxes)
{
	UWorld* World = GetWorld();
	if (!bDrawDebugSnapshot || !World || GetNetMode() == NM_DedicatedServer)
	{
		return;   // 렌더 없는 데디 서버는 스킵
	}

	for (const FHitBoxSnapshot& Box : Boxes)
	{
		DrawDebugBox(World, Box.Location, Box.BoxExtent, Box.Rotation.Quaternion(),
			FColor::Green, false, DebugDrawInterval, 0, 0.5f);   // 다음 전송까지 유지
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

	// QueryTime을 감싸는 두 스냅샷을 찾는다 (History는 Time 오름차순, [0]=가장 오래됨)
	int32 AfterIdx = INDEX_NONE;
	for (int32 i = 0; i < History.Num(); ++i)
	{
		if (History[i].Time >= QueryTime)
		{
			AfterIdx = i;
			break;
		}
	}

	const FServerSideRewindSnapshot* Before = nullptr;
	const FServerSideRewindSnapshot* After  = nullptr;
	float Alpha = 0.0f;

	if (AfterIdx == INDEX_NONE)
	{
		// QueryTime이 모든 스냅샷보다 최신 → 최신 스냅샷 사용 (보간 없음)
		Before = After = &History.Last();
	}
	else if (AfterIdx == 0)
	{
		// QueryTime이 모든 스냅샷보다 이전 → 가장 오래된 스냅샷 사용 (보간 없음)
		Before = After = &History[0];
	}
	else
	{
		Before = &History[AfterIdx - 1];
		After  = &History[AfterIdx];
		const float Span = After->Time - Before->Time;
		Alpha = (Span > SMALL_NUMBER) ? (QueryTime - Before->Time) / Span : 0.0f;
	}

	// Before의 각 히트박스를 같은 FName 키의 After 박스와 보간해 판정
	for (const TPair<FName, FHitBoxSnapshot>& Pair : Before->HitBoxSnapshots)
	{
		const FHitBoxSnapshot* BoxB = After->HitBoxSnapshots.Find(Pair.Key);
		const FHitBoxSnapshot Box = (BoxB && Alpha > 0.0f)
			? InterpolateBox(Pair.Value, *BoxB, Alpha)
			: Pair.Value;   // 경계/동일 프레임 → 보간 없이 사용

		if (SegmentIntersectsBox(Box, TraceStart, TraceEnd))
		{
			return true;
		}
	}
	return false;
}

