// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSRopeActionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Character/Components/LSCharacterMovementComponent.h"
#include "Character/LSPlayerCharacter.h"

ULSRopeActionComponent::ULSRopeActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULSRopeActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 프리뷰 판정은 소유 클라만 (서버/타 클라는 불필요)
	ACharacter* Character = GetOwnerCharacter();
	if (!Character || !Character->IsLocallyControlled())
	{
		return;
	}

	// 서버 재현 가능하도록 캐릭터 상태(눈 위치 + 컨트롤 회전)만 사용
	const FVector Start = Character->GetPawnViewLocation();
	const FVector Direction = Character->GetControlRotation().Vector();
	FindRopeTarget(Start, Direction, CurrentTarget);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugTarget(Start, CurrentTarget);
	}
#endif
}

bool ULSRopeActionComponent::FindRopeTarget(const FVector& Start, const FVector& Direction, FLSRopeTarget& OutTarget) const
{
	OutTarget = FLSRopeTarget();

	const ACharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		return false;
	}

	const FVector End = Start + Direction.GetSafeNormal() * MaxRange;
	OutTarget.SweepEnd = End;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RopeTrace), false, Character);
	FHitResult Hit;
	if (!GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, RopeTraceChannel, FCollisionShape::MakeSphere(TraceRadius), Params))
	{
		return false;
	}

	OutTarget.SweepEnd = Hit.Location;
	OutTarget.AnchorPoint = Hit.ImpactPoint;
	OutTarget.ImpactNormal = Hit.ImpactNormal;

	// 바닥면: 맞은 지점이 착지 후보 / 벽면: 모서리 위 탐색
	const bool bFloorSurface = Hit.ImpactNormal.Z >= Character->GetCharacterMovement()->GetWalkableFloorZ();

	FVector Candidate = FVector::ZeroVector;
	bool bHasCandidate = false;
	if (bFloorSurface)
	{
		Candidate = Hit.ImpactPoint;
		bHasCandidate = true;
	}
	else
	{
		bHasCandidate = FindLedgeLanding(Hit, Candidate);
	}

	if (bHasCandidate && HasStandingRoom(Candidate, OutTarget.LandingLocation))
	{
		OutTarget.Type = ELSRopeTargetType::Landing;
		return true;
	}

	// 설 자리 없음 / 벽: 앵커가 충분히 높을 때만 스윙 (발밑 앵커 방지)
	if (OutTarget.AnchorPoint.Z - Character->GetActorLocation().Z < MinSwingHeight)
	{
		return false;
	}

	OutTarget.Type = ELSRopeTargetType::Swing;
	return true;
}

bool ULSRopeActionComponent::FindLedgeLanding(const FHitResult& WallHit, FVector& OutCandidate) const
{
	const ACharacter* Character = GetOwnerCharacter();

	// 벽 안쪽 + 위로 올린 지점에서 명중 높이까지 아래로 탐색
	// 벽이 탐색 높이보다 높으면 시작점이 벽 내부라 명중 없음 → 모서리 없음
	const FVector WallNormal2D = FVector(WallHit.ImpactNormal.X, WallHit.ImpactNormal.Y, 0.0f).GetSafeNormal();
	const FVector DownStart = WallHit.ImpactPoint - WallNormal2D * LedgeForwardOffset + FVector::UpVector * LedgeSearchHeight;
	const FVector DownEnd = DownStart - FVector::UpVector * LedgeSearchHeight;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RopeLedgeTrace), false, Character);
	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, DownStart, DownEnd, RopeTraceChannel, Params))
	{
		return false;
	}

	// 모서리 윗면이 걸을 수 있는 경사여야 함
	if (Hit.ImpactNormal.Z < Character->GetCharacterMovement()->GetWalkableFloorZ())
	{
		return false;
	}

	OutCandidate = Hit.ImpactPoint;
	return true;
}

bool ULSRopeActionComponent::HasStandingRoom(const FVector& Candidate, FVector& OutLandingLocation) const
{
	const ACharacter* Character = GetOwnerCharacter();
	const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const float Radius = Capsule->GetScaledCapsuleRadius();

	// 착지 시 캡슐 중심 = 바닥 + 반높이 + 여유
	const FVector Location = Candidate + FVector::UpVector * (HalfHeight + LandingClearance);

	// 캡슐 오브젝트 타입 채널로 검사 → 실제 이동 충돌과 일치
	FCollisionQueryParams Params(SCENE_QUERY_STAT(RopeLandingOverlap), false, Character);
	if (GetWorld()->OverlapBlockingTestByChannel(Location, FQuat::Identity, Capsule->GetCollisionObjectType(), FCollisionShape::MakeCapsule(Radius, HalfHeight), Params))
	{
		return false;
	}

	OutLandingLocation = Location;
	return true;
}

void ULSRopeActionComponent::TryRopeAction()
{
	// 스윙 중 재입력 = 취소
	ULSCharacterMovementComponent* CMC = GetLSMovement();
	if (CMC && CMC->IsSwinging())
	{
		CMC->RequestStopSwing();
		return;
	}

	switch (CurrentTarget.Type)
	{
	case ELSRopeTargetType::Landing:
		StartLandingJump(CurrentTarget);
		break;
	case ELSRopeTargetType::Swing:
		StartSwing(CurrentTarget);
		break;
	default:
		break;
	}
}

void ULSRopeActionComponent::StartLandingJump(const FLSRopeTarget& Target)
{
	ALSPlayerCharacter* Player = Cast<ALSPlayerCharacter>(GetOwner());
	ULSCharacterMovementComponent* CMC = GetLSMovement();
	if (!Player || !CMC || !CMC->CanRopeJump())
	{
		return;
	}

	// 반응성: 소유 클라 몽타주 즉시 재생 → CMC 플래그로 서버 전달
	Player->PlayRopeJumpMontage();
	CMC->RequestRopeJump(Target.LandingLocation);
}

void ULSRopeActionComponent::StartSwing(const FLSRopeTarget& Target)
{
	ULSCharacterMovementComponent* CMC = GetLSMovement();
	if (!CMC || !CMC->CanStartSwing())
	{
		return;
	}

	// CMC 플래그 + 무브 데이터(앵커)로 서버 전달, 커스텀 무브먼트 모드에서 예측 이동
	CMC->RequestStartSwing(Target.AnchorPoint);
}

void ULSRopeActionComponent::DrawDebugTarget(const FVector& Start, const FLSRopeTarget& Target) const
{
#if ENABLE_DRAW_DEBUG
	const UWorld* World = GetWorld();
	const FColor Color =
		Target.Type == ELSRopeTargetType::Landing ? FColor::Green :
		Target.Type == ELSRopeTargetType::Swing ? FColor::Blue : FColor::Red;

	// 스윕 경로 + 도착 지점 구체 (명중 여부와 무관하게 항상)
	DrawDebugLine(World, Start, Target.SweepEnd, Color, false, -1.0f, 0, 1.0f);
	DrawDebugSphere(World, Target.SweepEnd, TraceRadius, 12, Color, false, -1.0f, 0, 1.0f);

	// 스윙 중 로프 (캐릭터 → 앵커)
	const ULSCharacterMovementComponent* CMC = GetLSMovement();
	if (CMC && CMC->IsSwinging())
	{
		DrawDebugLine(World, GetOwner()->GetActorLocation(), CMC->GetSwingAnchor(), FColor::Yellow, false, -1.0f, 0, 2.0f);
	}

	if (Target.Type == ELSRopeTargetType::None)
	{
		return;
	}

	// 앵커 지점
	DrawDebugPoint(World, Target.AnchorPoint, 12.0f, Color, false, -1.0f);

	// 착지 캡슐
	if (Target.Type == ELSRopeTargetType::Landing)
	{
		if (const ACharacter* Character = GetOwnerCharacter())
		{
			const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
			DrawDebugCapsule(World, Target.LandingLocation, Capsule->GetScaledCapsuleHalfHeight(), Capsule->GetScaledCapsuleRadius(), FQuat::Identity, Color, false, -1.0f, 0, 1.0f);
		}
	}
#endif
}

ACharacter* ULSRopeActionComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

ULSCharacterMovementComponent* ULSRopeActionComponent::GetLSMovement() const
{
	const ACharacter* Character = GetOwnerCharacter();
	return Character ? Cast<ULSCharacterMovementComponent>(Character->GetCharacterMovement()) : nullptr;
}
