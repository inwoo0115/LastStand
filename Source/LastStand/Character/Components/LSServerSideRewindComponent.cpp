// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSServerSideRewindComponent.h"
#include "Character/Components/LSHitboxComponent.h"
#include "Interface/LSHitboxInterface.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

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

	// 판정 대상 설정: 클라 예측 트레이스도 판정 방식에 맞아야 하므로 충돌 설정은 모든 머신에서 수행
	if (Owner && Owner->Implements<ULSHitboxInterface>())
	{
		ILSHitboxInterface* HitboxOwner = Cast<ILSHitboxInterface>(Owner);

		TArray<ULSHitboxComponent*> Found;
		HitboxOwner->GetHitboxComponents(Found);

		if (bUsePhysicsAsset)
		{
			// 물리 에셋 버전: 남아 있는 히트박스는 트레이스를 가로채지 않도록 충돌을 끔
			for (ULSHitboxComponent* Hitbox : Found)
			{
				if (Hitbox)
				{
					Hitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
			}

			// 메쉬 충돌을 켜 물리 에셋 바디 생성 (NoCollision이면 바디가 생성되지 않음)
			USkeletalMeshComponent* Mesh = HitboxOwner->GetHitboxMesh();
			if (Mesh && Mesh->GetPhysicsAsset())
			{
				Mesh->SetCollisionProfileName(BodyCollisionProfileName);
				CachedMesh = Mesh;
			}
		}
		else
		{
			// 히트박스 버전: 소유 pawn의 전체 히트박스 수집 (개수 가변)
			CachedHitboxes.Reset();
			for (ULSHitboxComponent* Hitbox : Found)
			{
				CachedHitboxes.Add(Hitbox);
			}
		}
	}

	// 히스토리는 서버에서만 의미가 있으므로 클라에서는 기록 안 함
	if (!Owner || !Owner->HasAuthority())
	{
		SetComponentTickEnabled(false);
		return;
	}

	// 판정 대상이 없으면 이 적은 맞지 않음
	const bool bNoTarget = bUsePhysicsAsset ? (!CachedMesh || CachedMesh->Bodies.Num() == 0) : CachedHitboxes.Num() == 0;
	if (bNoTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("SSR: %s has no %s for rewind"), *GetNameSafe(Owner),
			bUsePhysicsAsset ? TEXT("physics bodies") : TEXT("hitboxes"));
	}
}


// Called every frame
void ULSServerSideRewindComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UWorld* World = GetWorld();
	const bool bNoTarget = bUsePhysicsAsset ? !CachedMesh : CachedHitboxes.Num() == 0;
	if (!World || bNoTarget)
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

	// 현재 시점의 판정 대상 스냅샷 기록
	FServerSideRewindSnapshot Snapshot;
	Snapshot.Time = Now;
	if (bUsePhysicsAsset)
	{
		// 물리 에셋 버전: 바디 월드 트랜스폼 (null 바디는 Identity로 인덱스 유지)
		Snapshot.BodyTransforms.Reserve(CachedMesh->Bodies.Num());
		for (const FBodyInstance* Body : CachedMesh->Bodies)
		{
			Snapshot.BodyTransforms.Add(Body ? Body->GetUnrealWorldTransform() : FTransform::Identity);
		}
	}
	else
	{
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
	}
	History.Add(MoveTemp(Snapshot));

	// End(200ms)보다 오래된 스냅샷을 front에서 제거 (큐 FIFO 만료)
	while (History.Num() > 0 && (Now - History[0].Time) > HistoryEndOffset)
	{
		History.RemoveAt(0);
	}

#if WITH_EDITOR
	// 히트박스 버전 전용. 물리 에셋 버전은 엔진 show Collision으로 실제 바디 형상 확인
	if (bDrawDebugSnapshot && !bUsePhysicsAsset && Now - LastDebugDrawTime >= DebugDrawInterval)
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
#endif
}


void ULSServerSideRewindComponent::MulticastDrawDebugBoxes_Implementation(const TArray<FHitBoxSnapshot>& Boxes)
{
	// 에디터 전용 — 선언은 RPC 테이블 일치를 위해 모든 빌드에 유지하고 본문만 제외
#if WITH_EDITOR
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
#endif
}


bool ULSServerSideRewindComponent::ConfirmHit(const FVector& TraceStart, const FVector& TraceEnd, const APawn* Shooter, ULSHitboxComponent*& OutHitbox) const
{
	OutHitbox = nullptr;

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

	// 서버가 패킷 ACK로 측정한 쏜 사람의 RTT(초, 최근 ~4초 평균). 클라 값은 신뢰하지 않음.
	// 리슨 서버 호스트/정보 없음 → 0(현재 시점 판정)
	float RewindTime = 0.0f;
	if (Shooter)
	{
		if (const APlayerState* PS = Shooter->GetPlayerState())
		{
			RewindTime = PS->GetPingInMilliseconds() * 0.001f;
		}
	}

	// 클라 화면의 적은 RTT/2 과거 + 발사 RPC 도착까지 RTT/2 → 총 RTT만큼 되돌림. 200ms 상한
	const float QueryTime = Now - FMath::Clamp(RewindTime, 0.0f, HistoryEndOffset);

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

	// 판정 방식별 처리
	if (bUsePhysicsAsset)
	{
		return CachedMesh && ConfirmBodyHit(*Before, *After, Alpha, TraceStart, TraceEnd);
	}

	OutHitbox = ConfirmHitboxHit(*Before, *After, Alpha, TraceStart, TraceEnd);
	return OutHitbox != nullptr;
}


ULSHitboxComponent* ULSServerSideRewindComponent::ConfirmHitboxHit(const FServerSideRewindSnapshot& Before, const FServerSideRewindSnapshot& After, float Alpha, const FVector& TraceStart, const FVector& TraceEnd) const
{
	// Before의 각 히트박스를 같은 FName 키의 After 박스와 보간해 판정
	for (const TPair<FName, FHitBoxSnapshot>& Pair : Before.HitBoxSnapshots)
	{
		const FHitBoxSnapshot* BoxB = After.HitBoxSnapshots.Find(Pair.Key);
		const FHitBoxSnapshot Box = (BoxB && Alpha > 0.0f)
			? InterpolateBox(Pair.Value, *BoxB, Alpha)
			: Pair.Value;   // 경계/동일 프레임 → 보간 없이 사용

		if (SegmentIntersectsBox(Box, TraceStart, TraceEnd))
		{
			// 교차한 스냅샷의 FName에 해당하는 라이브 히트박스를 반환
			for (ULSHitboxComponent* Hitbox : CachedHitboxes)
			{
				if (Hitbox && Hitbox->GetFName() == Pair.Key)
				{
					return Hitbox;
				}
			}
			return nullptr;
		}
	}
	return nullptr;
}


bool ULSServerSideRewindComponent::ConfirmBodyHit(const FServerSideRewindSnapshot& Before, const FServerSideRewindSnapshot& After, float Alpha, const FVector& TraceStart, const FVector& TraceEnd) const
{
	const TArray<FBodyInstance*>& Bodies = CachedMesh->Bodies;
	if (Before.BodyTransforms.Num() != Bodies.Num() || After.BodyTransforms.Num() != Bodies.Num())
	{
		return false;   // 스냅샷 이후 물리 상태가 재생성된 경우 (인덱스 짝 불일치)
	}

	// 1) 현재 포즈를 보관하고 바디를 쿼리 시점 포즈로 이동 (TeleportPhysics: 게임 스레드 위치 즉시 반영)
	TArray<FTransform, TInlineAllocator<32>> CurrentTransforms;
	CurrentTransforms.Reserve(Bodies.Num());
	for (int32 i = 0; i < Bodies.Num(); ++i)
	{
		FBodyInstance* Body = Bodies[i];
		CurrentTransforms.Add(Body ? Body->GetUnrealWorldTransform() : FTransform::Identity);
		if (Body)
		{
			FTransform RewoundTM = Before.BodyTransforms[i];
			if (Alpha > 0.0f)
			{
				RewoundTM.Blend(Before.BodyTransforms[i], After.BodyTransforms[i], Alpha);
			}
			Body->SetBodyTransform(RewoundTM, ETeleportType::TeleportPhysics);
		}
	}

	// 2) 되돌린 포즈로 메쉬 바디만 트레이스 (가장 가까운 바디 명중)
	FHitResult Hit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(LSRewindTrace), false);
	const bool bHit = CachedMesh->LineTraceComponent(Hit, TraceStart, TraceEnd, Params);

	// 3) 원래 포즈로 복원 (명중 여부와 무관하게 항상)
	for (int32 i = 0; i < Bodies.Num(); ++i)
	{
		if (FBodyInstance* Body = Bodies[i])
		{
			Body->SetBodyTransform(CurrentTransforms[i], ETeleportType::TeleportPhysics);
		}
	}
	return bHit;
}
