// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/RootMotionSource.h"
#include "Character/LSPlayerCharacter.h"
#include "Character/Components/LSActionStateComponent.h"
#include "Tags/LSGameplayTags.h"
#include "Character/Components/RootMotion/LSRootMotionSource.h"
#include "Character/Components/LSRopeActionComponent.h"

namespace
{
	const FName DashRootMotionName(TEXT("LSDash"));
	const FName RopeJumpRootMotionName(TEXT("LSRopeJump"));
}

// ---------- FSavedMove_LS ----------
void FSavedMove_LS::Clear()
{
	Super::Clear();

	bSavedWantsToRun = false;
	bSavedWantsToDash = false;
	bSavedWantsToRopeJump = false;
	SavedRopeJumpTarget = FVector::ZeroVector;
	SavedRopeJumpDuration = 0.0f;
	SavedRopeJumpArcHeight = 0.0f;
	bSavedWantsToStartSwing = false;
	bSavedWantsToStopSwing = false;
	SavedSwingRequestAnchor = FVector::ZeroVector;
	SavedSwingAnchor = FVector::ZeroVector;
}

uint8 FSavedMove_LS::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedWantsToRun)
	{
		Result |= FLAG_Custom_0;
	}
	if (bSavedWantsToDash)
	{
		Result |= FLAG_Custom_1;
	}
	if (bSavedWantsToRopeJump)
	{
		Result |= FLAG_Custom_2;
	}
	if (bSavedWantsToStartSwing || bSavedWantsToStopSwing)
	{
		Result |= FLAG_Custom_3;
	}
	return Result;
}

bool FSavedMove_LS::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_LS* NewLSMove = static_cast<FSavedMove_LS*>(NewMove.Get());

	if (bSavedWantsToRun != NewLSMove->bSavedWantsToRun || bSavedWantsToDash || NewLSMove->bSavedWantsToDash
		|| bSavedWantsToRopeJump || NewLSMove->bSavedWantsToRopeJump
		|| bSavedWantsToStartSwing || NewLSMove->bSavedWantsToStartSwing
		|| bSavedWantsToStopSwing || NewLSMove->bSavedWantsToStopSwing)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_LS::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if (const ULSCharacterMovementComponent* CMC = Cast<ULSCharacterMovementComponent>(C->GetCharacterMovement()))
	{
		bSavedWantsToRun = CMC->bWantsToRun;
		bSavedWantsToDash = CMC->bWantsToDash;
		bSavedWantsToRopeJump = CMC->bWantsToRopeJump;
		SavedRopeJumpTarget = CMC->RopeJumpTarget;
		SavedRopeJumpDuration = CMC->RopeJumpDuration;
		SavedRopeJumpArcHeight = CMC->RopeJumpArcHeight;
		bSavedWantsToStartSwing = CMC->bWantsToStartSwing;
		bSavedWantsToStopSwing = CMC->bWantsToStopSwing;
		SavedSwingRequestAnchor = CMC->SwingRequestAnchor;
		SavedSwingAnchor = CMC->SwingAnchor;
	}
}

void FSavedMove_LS::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	if (ULSCharacterMovementComponent* CMC = Cast<ULSCharacterMovementComponent>(C->GetCharacterMovement()))
	{
		CMC->bWantsToRun = bSavedWantsToRun;
		CMC->bWantsToDash = bSavedWantsToDash;
		CMC->bWantsToRopeJump = bSavedWantsToRopeJump;
		CMC->RopeJumpTarget = SavedRopeJumpTarget;
		CMC->RopeJumpDuration = SavedRopeJumpDuration;
		CMC->RopeJumpArcHeight = SavedRopeJumpArcHeight;
		CMC->bWantsToStartSwing = bSavedWantsToStartSwing;
		CMC->bWantsToStopSwing = bSavedWantsToStopSwing;
		CMC->SwingRequestAnchor = SavedSwingRequestAnchor;
		CMC->SwingAnchor = SavedSwingAnchor;
	}
}

// ---------- FNetworkPredictionData_Client_LS ----------
FNetworkPredictionData_Client_LS::FNetworkPredictionData_Client_LS(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_LS::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_LS());
}

// ---------- FLSCharacterNetworkMoveData ----------
void FLSCharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(ClientMove, MoveType);

	const FSavedMove_LS& LSMove = static_cast<const FSavedMove_LS&>(ClientMove);
	RopeJumpTarget = LSMove.SavedRopeJumpTarget;
	RopeJumpDuration = LSMove.SavedRopeJumpDuration;
	RopeJumpArcHeight = LSMove.SavedRopeJumpArcHeight;
	bSwingRequestStart = LSMove.bSavedWantsToStartSwing;
	SwingRequestAnchor = LSMove.SavedSwingRequestAnchor;
}

bool FLSCharacterNetworkMoveData::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	Super::Serialize(CharacterMovement, Ar, PackageMap, MoveType);

	if (CompressedMoveFlags & FSavedMove_Character::FLAG_Custom_2)
	{
		bool bSuccess = true;
		RopeJumpTarget.NetSerialize(Ar, PackageMap, bSuccess);
		Ar << RopeJumpDuration;
		Ar << RopeJumpArcHeight;
	}

	if (CompressedMoveFlags & FSavedMove_Character::FLAG_Custom_3)
	{
		Ar.SerializeBits(&bSwingRequestStart, 1);
		if (bSwingRequestStart)
		{
			bool bSuccess = true;
			SwingRequestAnchor.NetSerialize(Ar, PackageMap, bSuccess);
		}
	}

	return !Ar.IsError();
}

// ---------- FLSCharacterNetworkMoveDataContainer ----------
FLSCharacterNetworkMoveDataContainer::FLSCharacterNetworkMoveDataContainer()
{
	NewMoveData = &MoveData[0];
	PendingMoveData = &MoveData[1];
	OldMoveData = &MoveData[2];
}

// ---------- ULSCharacterMovementComponent ----------
ULSCharacterMovementComponent::ULSCharacterMovementComponent()
{
	SetNetworkMoveDataContainer(LSMoveDataContainer);
}

float ULSCharacterMovementComponent::GetMaxSpeed() const
{
	// 지상 이동 중 달리기면 달리기 속도 (MaxWalkSpeed는 직접 변경하지 않음)
	if (bWantsToRun && MovementMode == MOVE_Walking)
	{
		return MaxRunSpeed;
	}
	return Super::GetMaxSpeed();
}

FNetworkPredictionData_Client* ULSCharacterMovementComponent::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		ULSCharacterMovementComponent* MutableThis = const_cast<ULSCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_LS(*this);
	}
	return ClientPredictionData;
}

void ULSCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToRun = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bWantsToDash = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
	bWantsToRopeJump = (Flags & FSavedMove_Character::FLAG_Custom_2) != 0;

	if (bWantsToRopeJump && CharacterOwner->HasAuthority())
	{
		if (const FLSCharacterNetworkMoveData* MoveData = static_cast<const FLSCharacterNetworkMoveData*>(GetCurrentNetworkMoveData()))
		{
			RopeJumpTarget = MoveData->RopeJumpTarget;
			RopeJumpDuration = MoveData->RopeJumpDuration;
			RopeJumpArcHeight = MoveData->RopeJumpArcHeight;
		}
	}

	const bool bSwingRequest = (Flags & FSavedMove_Character::FLAG_Custom_3) != 0;
	if (CharacterOwner->HasAuthority())
	{
		bWantsToStartSwing = false;
		bWantsToStopSwing = false;
		if (bSwingRequest)
		{
			if (const FLSCharacterNetworkMoveData* MoveData = static_cast<const FLSCharacterNetworkMoveData*>(GetCurrentNetworkMoveData()))
			{
				bWantsToStartSwing = MoveData->bSwingRequestStart;
				bWantsToStopSwing = !MoveData->bSwingRequestStart;
				SwingRequestAnchor = MoveData->SwingRequestAnchor;
			}
		}
	}
}

bool ULSCharacterMovementComponent::ClientUpdatePositionAfterServerUpdate()
{
	const bool bRealWantsToRun = bWantsToRun;
	const bool bRealWantsToDash = bWantsToDash;
	const bool bRealWantsToRopeJump = bWantsToRopeJump;
	const FVector RealRopeJumpTarget = RopeJumpTarget;
	const float RealRopeJumpDuration = RopeJumpDuration;
	const float RealRopeJumpArcHeight = RopeJumpArcHeight;
	const bool bRealWantsToStartSwing = bWantsToStartSwing;
	const bool bRealWantsToStopSwing = bWantsToStopSwing;
	const FVector RealSwingRequestAnchor = SwingRequestAnchor;

	const bool bResult = Super::ClientUpdatePositionAfterServerUpdate();

	bWantsToRun = bRealWantsToRun;
	bWantsToDash = bRealWantsToDash;
	bWantsToRopeJump = bRealWantsToRopeJump;
	RopeJumpTarget = RealRopeJumpTarget;
	RopeJumpDuration = RealRopeJumpDuration;
	RopeJumpArcHeight = RealRopeJumpArcHeight;
	bWantsToStartSwing = bRealWantsToStartSwing;
	bWantsToStopSwing = bRealWantsToStopSwing;
	SwingRequestAnchor = RealSwingRequestAnchor;
	return bResult;
}

void ULSCharacterMovementComponent::RequestRopeJump(const FVector& Target)
{
	const FVector Start = UpdatedComponent->GetComponentLocation();

	bWantsToRopeJump = true;
	RopeJumpTarget = FVector(
		FMath::RoundToFloat(Target.X * 10.0f) / 10.0f,
		FMath::RoundToFloat(Target.Y * 10.0f) / 10.0f,
		FMath::RoundToFloat(Target.Z * 10.0f) / 10.0f);
	RopeJumpDuration = FMath::Clamp(FVector::Dist(Start, RopeJumpTarget) / RopeJumpSpeed, RopeJumpMinDuration, RopeJumpMaxDuration);
	RopeJumpArcHeight = RopeJumpBaseArcHeight + FMath::Max(RopeJumpTarget.Z - Start.Z, 0.0f) * 0.5f;
}

bool ULSCharacterMovementComponent::CanRopeJump() const
{
	if (IsRoping() || IsDashing() || IsSwinging())
	{
		return false;
	}

	if (!CharacterOwner->HasAuthority())
	{
		return true;
	}

	if (RopeJumpDuration < RopeJumpMinDuration || RopeJumpDuration > RopeJumpMaxDuration
		|| RopeJumpArcHeight < 0.0f || RopeJumpArcHeight > RopeJumpMaxArcHeight)
	{
		return false;
	}

	const ULSRopeActionComponent* Rope = CharacterOwner->FindComponentByClass<ULSRopeActionComponent>();
	if (!Rope)
	{
		return false;
	}

	FLSRopeTarget ServerTarget;
	const bool bFound = Rope->FindRopeTarget(CharacterOwner->GetPawnViewLocation(), CharacterOwner->GetControlRotation().Vector(), ServerTarget);
	return bFound
		&& ServerTarget.Type == ELSRopeTargetType::Landing
		&& FVector::Dist(ServerTarget.LandingLocation, RopeJumpTarget) <= RopeJumpValidationTolerance;
}

bool ULSCharacterMovementComponent::IsRoping() const
{
	return HasRootMotionSourceNamed(RopeJumpRootMotionName);
}

bool ULSCharacterMovementComponent::HasRootMotionSourceNamed(FName Name) const
{
	auto HasSource = [&Name](const TArray<TSharedPtr<FRootMotionSource>>& Sources)
	{
		return Sources.ContainsByPredicate([&Name](const TSharedPtr<FRootMotionSource>& Source)
		{
			return Source.IsValid() && Source->InstanceName == Name;
		});
	};
	return HasSource(CurrentRootMotion.RootMotionSources) || HasSource(CurrentRootMotion.PendingAddRootMotionSources);
}

bool ULSCharacterMovementComponent::CanDash() const
{
	if (!IsMovingOnGround() || GetDashDirection().IsNearlyZero())
	{
		return false;
	}

	// 대시 진행 중이면 불가
	if (IsDashing())
	{
		return false;
	}

	// 쿨다운: 서버는 지연 허용오차 0.9 (연사 가드와 동일 방식)
	const float Cooldown = CharacterOwner && CharacterOwner->HasAuthority() ? DashCooldown * 0.9f : DashCooldown;
	return GetWorld()->GetTimeSeconds() - LastDashTime >= Cooldown;
}

bool ULSCharacterMovementComponent::IsDashing() const
{
	return HasRootMotionSourceNamed(DashRootMotionName);
}

ELSDashDirection ULSCharacterMovementComponent::GetDashDirectionType(const FVector& Direction) const
{
	// 액터 기준 로컬 방향에서 큰 축으로 4방향 결정
	const FVector Local = UpdatedComponent->GetComponentRotation().UnrotateVector(Direction);
	if (FMath::Abs(Local.X) >= FMath::Abs(Local.Y))
	{
		return Local.X >= 0.0f ? ELSDashDirection::Forward : ELSDashDirection::Backward;
	}
	return Local.Y >= 0.0f ? ELSDashDirection::Right : ELSDashDirection::Left;
}

void ULSCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	if (bWantsToDash && !CharacterOwner->bClientUpdating && CanDash())
	{
		PerformDash();
	}
	bWantsToDash = false;

	if (bWantsToRopeJump && !CharacterOwner->bClientUpdating && CanRopeJump())
	{
		PerformRopeJump();
	}
	bWantsToRopeJump = false;

	if (bWantsToStopSwing && IsSwinging())
	{
		StopSwing();
	}
	else if (bWantsToStartSwing && CanStartSwing())
	{
		StartSwing();
	}
	bWantsToStartSwing = false;
	bWantsToStopSwing = false;
}

void ULSCharacterMovementComponent::PerformDash()
{
	const FVector Direction = GetDashDirection();

	// 일정 속도로 DashDuration 동안 이동, 종료 시 걷기 속도로 클램프
	TSharedPtr<FRootMotionSource_ConstantForce> DashForce = MakeShared<FRootMotionSource_ConstantForce>();
	DashForce->InstanceName = DashRootMotionName;
	DashForce->AccumulateMode = ERootMotionAccumulateMode::Override;
	DashForce->Priority = 5;
	DashForce->Force = Direction * DashSpeed;
	DashForce->Duration = DashDuration;
	DashForce->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::ClampVelocity;
	DashForce->FinishVelocityParams.ClampVelocity = MaxWalkSpeed;
	ApplyRootMotionSource(DashForce);

	LastDashTime = GetWorld()->GetTimeSeconds();

	if (CharacterOwner->HasAuthority())
	{
		if (ALSPlayerCharacter* Player = Cast<ALSPlayerCharacter>(CharacterOwner))
		{
			if (ULSActionStateComponent* ActionState = Player->GetActionStateComponent())
			{
				ActionState->CancelStatesFor(LSActionTags::Action_Dash);
			}
			Player->MulticastRPCPlayDashMontage(GetDashDirectionType(Direction));
		}
	}
}

void ULSCharacterMovementComponent::PerformRopeJump()
{
	TSharedPtr<FLSRootMotionSource_ParabolicMoveTo> JumpForce = MakeShared<FLSRootMotionSource_ParabolicMoveTo>();
	JumpForce->InstanceName = RopeJumpRootMotionName;
	JumpForce->AccumulateMode = ERootMotionAccumulateMode::Override;
	JumpForce->Priority = 5;
	JumpForce->StartLocation = UpdatedComponent->GetComponentLocation();
	JumpForce->TargetLocation = RopeJumpTarget;
	JumpForce->ArcHeight = RopeJumpArcHeight;
	JumpForce->Duration = RopeJumpDuration;
	JumpForce->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	JumpForce->FinishVelocityParams.SetVelocity = FVector::ZeroVector;
	ApplyRootMotionSource(JumpForce);

	if (CharacterOwner->HasAuthority())
	{
		if (ALSPlayerCharacter* Player = Cast<ALSPlayerCharacter>(CharacterOwner))
		{
			Player->MulticastRPCPlayRopeJumpMontage();
		}
	}
}

void ULSCharacterMovementComponent::RequestStartSwing(const FVector& Anchor)
{
	bWantsToStartSwing = true;
	SwingRequestAnchor = FVector(
		FMath::RoundToFloat(Anchor.X * 10.0f) / 10.0f,
		FMath::RoundToFloat(Anchor.Y * 10.0f) / 10.0f,
		FMath::RoundToFloat(Anchor.Z * 10.0f) / 10.0f);
}

bool ULSCharacterMovementComponent::IsSwinging() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ELSCustomMovementMode::Swing);
}

bool ULSCharacterMovementComponent::CanStartSwing() const
{
	if (IsSwinging() || IsRoping() || IsDashing())
	{
		return false;
	}

	if (!CharacterOwner->HasAuthority())
	{
		return true;
	}

	const ULSRopeActionComponent* Rope = CharacterOwner->FindComponentByClass<ULSRopeActionComponent>();
	if (!Rope)
	{
		return false;
	}

	FLSRopeTarget ServerTarget;
	const bool bFound = Rope->FindRopeTarget(CharacterOwner->GetPawnViewLocation(), CharacterOwner->GetControlRotation().Vector(), ServerTarget);
	return bFound
		&& ServerTarget.Type == ELSRopeTargetType::Swing
		&& FVector::Dist(ServerTarget.AnchorPoint, SwingRequestAnchor) <= SwingValidationTolerance;
}

void ULSCharacterMovementComponent::StartSwing()
{
	SwingAnchor = SwingRequestAnchor;
	SetMovementMode(MOVE_Custom, static_cast<uint8>(ELSCustomMovementMode::Swing));
}

void ULSCharacterMovementComponent::StopSwing()
{
	SetMovementMode(MOVE_Falling);
}

void ULSCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (CustomMovementMode == static_cast<uint8>(ELSCustomMovementMode::Swing))
	{
		PhysSwing(deltaTime, Iterations);
	}

	Super::PhysCustom(deltaTime, Iterations);
}

void ULSCharacterMovementComponent::PhysSwing(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	// 루트모션 호환 (엔진 Phys* 관례): Override 루트모션이 있으면 스윙 물리 대신 적용
	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		// 이번 틱 로프 방향(캐릭터 → 앵커)과 거리
		const FVector ToAnchor = SwingAnchor - UpdatedComponent->GetComponentLocation();
		const float Distance = ToAnchor.Size();
		const FVector RopeDir = Distance > UE_KINDA_SMALL_NUMBER ? ToAnchor / Distance : FVector::ZeroVector;

		// 중력 (진자 운동 원동력)
		Velocity.Z += GetGravityZ() * deltaTime;

		// 방향키 입력: 수평 가속도 × 비율 (SavedMove로 서버 전달되는 값이라 추가 동기화 불필요)
		Velocity += ProjectToGravityFloor(Acceleration) * SwingInputAccelerationRatio * deltaTime;

		// 장력: 로프 길이(SwingRopeLength)를 초과한 만큼만 앵커로 당김 (느슨하면 자유낙하)
		const float Stretch = FMath::Max(Distance - SwingRopeLength, 0.0f);
		if (Stretch > 0.0f)
		{
			Velocity += RopeDir * (SwingTensionCoefficient * Stretch * deltaTime);

			// 감쇠: 로프 방향 성분만 감소 → 출렁임 억제, 좌우 스윙 속도는 유지
			const float RadialSpeed = FVector::DotProduct(Velocity, RopeDir);
			Velocity -= RopeDir * RadialSpeed * FMath::Clamp(SwingDamping * deltaTime, 0.0f, 1.0f);
		}

		// 입력 누적/과한 계수로 인한 폭주 방지
		Velocity = Velocity.GetClampedToMaxSize(SwingMaxSpeed);
	}

	ApplyRootMotionToVelocity(deltaTime);

	Iterations++;
	bJustTeleported = false;

	// 스윕 이동, 막히면 충돌면을 따라 미끄러짐 (스윙 유지)
	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Delta = Velocity * deltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, deltaTime, Delta);
		SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
	}

	// 실제 변위로 속도 재계산 → 충돌로 막힌 성분 제거 (엔진 PhysFlying과 동일)
	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}
}
