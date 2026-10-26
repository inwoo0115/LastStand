// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LSCharacterMovementComponent.generated.h"

// 대시 방향 (액터 기준)
UENUM(BlueprintType)
enum class ELSDashDirection : uint8
{
	Forward   UMETA(DisplayName = "Forward"),
	Backward  UMETA(DisplayName = "Backward"),
	Left      UMETA(DisplayName = "Left"),
	Right     UMETA(DisplayName = "Right")
};

class FSavedMove_LS : public FSavedMove_Character
{
public:
	using Super = FSavedMove_Character;

	uint8 bSavedWantsToRun : 1;
	uint8 bSavedWantsToDash : 1;

	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* C) override;
};

class FNetworkPredictionData_Client_LS : public FNetworkPredictionData_Client_Character
{
public:
	using Super = FNetworkPredictionData_Client_Character;

	FNetworkPredictionData_Client_LS(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};

// 플레이어 무브먼트: 달리기(속도) + 대시(Root Motion Source)
UCLASS()
class LASTSTAND_API ULSCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend class FSavedMove_LS;

public:
	virtual float GetMaxSpeed() const override;

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	// 달리기 토글 (소유 클라가 세팅 → 플래그로 서버 전달)
	void SetWantsToRun(bool bNewRun) { bWantsToRun = bNewRun; }

	// 대시 요청 (소유 클라). 다음 이동 틱에 처리
	void RequestDash() { bWantsToDash = true; }

	// 대시 가능 여부: 지상 + 입력 있음 + 대시 중 아님 + 쿨다운
	bool CanDash() const;

	// 현재 입력(가속) 기준 대시 방향 (수평 단위 벡터, 입력 없으면 Zero)
	FVector GetDashDirection() const { return Acceleration.GetSafeNormal2D(); }

	// 월드 방향 → 액터 기준 앞/뒤/좌/우
	ELSDashDirection GetDashDirectionType(const FVector& Direction) const;

	// 대시 루트모션 진행 중 여부 (중복 대시 방지, 추후 입력 차단에 사용 가능)
	bool IsDashing() const;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	// 리플레이가 SavedMove 값으로 플래그를 덮어쓰므로 이번 프레임 입력을 보존
	virtual bool ClientUpdatePositionAfterServerUpdate() override;

	void PerformDash();

	// 달리기
	UPROPERTY(EditAnywhere, Category = "Run", meta = (ClampMin = "0.0"))
	float MaxRunSpeed = 600.0f;

	// 대시
	UPROPERTY(EditAnywhere, Category = "Dash", meta = (ClampMin = "0.0"))
	float DashSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Dash", meta = (ClampMin = "0.01"))
	float DashDuration = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Dash", meta = (ClampMin = "0.0"))
	float DashCooldown = 1.0f;

	bool bWantsToRun = false;
	bool bWantsToDash = false;

	// 머신별 마지막 대시 시각 (쿨다운)
	float LastDashTime = -1000.0f;
};
