// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/LSCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/RootMotionSource.h"
#include "Character/LSPlayerCharacter.h"

namespace
{
	// 대시 루트모션 소스 식별용 (진행 중 확인)
	const FName DashRootMotionName(TEXT("LSDash"));
}

// ---------- FSavedMove_LS ----------
void FSavedMove_LS::Clear()
{
	Super::Clear();

	bSavedWantsToRun = false;
	bSavedWantsToDash = false;
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
	return Result;
}

bool FSavedMove_LS::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_LS* NewLSMove = static_cast<FSavedMove_LS*>(NewMove.Get());

	// 달리기 상태가 바뀌었거나 대시 요청이 있는 move는 합치지 않음 (요청 유실 방지)
	if (bSavedWantsToRun != NewLSMove->bSavedWantsToRun || bSavedWantsToDash || NewLSMove->bSavedWantsToDash)
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
	}
}

void FSavedMove_LS::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	// 리플레이 시 move 당시 상태 복원
	if (ULSCharacterMovementComponent* CMC = Cast<ULSCharacterMovementComponent>(C->GetCharacterMovement()))
	{
		CMC->bWantsToRun = bSavedWantsToRun;
		CMC->bWantsToDash = bSavedWantsToDash;
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

// ---------- ULSCharacterMovementComponent ----------
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

	// 서버: 클라 move 플래그 → 상태 복원
	bWantsToRun = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bWantsToDash = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
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
	// 이름으로 루트모션 소스 검색 (const 버전이 없어 직접 순회). 적용 직후엔 Pending 배열에 있음
	auto HasDash = [](const TArray<TSharedPtr<FRootMotionSource>>& Sources)
	{
		return Sources.ContainsByPredicate([](const TSharedPtr<FRootMotionSource>& Source)
		{
			return Source.IsValid() && Source->InstanceName == DashRootMotionName;
		});
	};
	return HasDash(CurrentRootMotion.RootMotionSources) || HasDash(CurrentRootMotion.PendingAddRootMotionSources);
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

	if (!bWantsToDash)
	{
		return;
	}
	bWantsToDash = false;

	// 리플레이(서버 보정 후 재시뮬)에서는 쿨다운 시각이 이미 갱신돼 있으므로 쿨다운 검사 생략
	const bool bReplaying = CharacterOwner && CharacterOwner->bClientUpdating;
	if (bReplaying ? (IsMovingOnGround() && !GetDashDirection().IsNearlyZero()) : CanDash())
	{
		PerformDash();
	}
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

	// 리플레이가 아닐 때만 쿨다운/연출 처리
	if (CharacterOwner && !CharacterOwner->bClientUpdating)
	{
		LastDashTime = GetWorld()->GetTimeSeconds();

		// 서버: 타 클라에 대시 몽타주 전파 (소유 클라는 입력 시 로컬 재생)
		if (CharacterOwner->HasAuthority())
		{
			if (ALSPlayerCharacter* Player = Cast<ALSPlayerCharacter>(CharacterOwner))
			{
				Player->MulticastRPCPlayDashMontage(GetDashDirectionType(Direction));
			}
		}
	}
}
