// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LSRopeActionComponent.generated.h"

// 로프 타깃 종류
UENUM(BlueprintType)
enum class ELSRopeTargetType : uint8
{
	None    UMETA(DisplayName = "None"),
	Landing UMETA(DisplayName = "Landing"),
	Swing   UMETA(DisplayName = "Swing")
};

// 로프 판정 결과
USTRUCT(BlueprintType)
struct FLSRopeTarget
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	ELSRopeTargetType Type = ELSRopeTargetType::None;

	// 로프가 걸리는 지점 (스윕 ImpactPoint)
	UPROPERTY(BlueprintReadOnly)
	FVector AnchorPoint = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector ImpactNormal = FVector::ZeroVector;

	// Landing일 때만: 착지 시 캡슐 중심 위치
	UPROPERTY(BlueprintReadOnly)
	FVector LandingLocation = FVector::ZeroVector;

	// 스윕 도착 지점 (명중 시 구체가 멈춘 중심, 미명중 시 최대 사거리 지점)
	UPROPERTY(BlueprintReadOnly)
	FVector SweepEnd = FVector::ZeroVector;
};

// 컨트롤 방향 구체 스윕으로 로프 타깃(착지/스윙) 판정 + 입력 시 타입별 분기
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LASTSTAND_API ULSRopeActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULSRopeActionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 시작점/방향만으로 판정 (서버 재검증에서도 같은 입력으로 호출 가능). 유효 타깃이면 true
	bool FindRopeTarget(const FVector& Start, const FVector& Direction, FLSRopeTarget& OutTarget) const;

	const FLSRopeTarget& GetCurrentTarget() const { return CurrentTarget; }

	// 입력 시 호출: 현재 타깃 타입별 동작 분기
	void TryRopeAction();

protected:
	// 타입별 동작: Landing = 포물선 루트모션, Swing = 커스텀 무브먼트 모드
	void StartLandingJump(const FLSRopeTarget& Target);
	void StartSwing(const FLSRopeTarget& Target);

	// 벽면 명중 시 모서리 위 착지 후보 탐색
	bool FindLedgeLanding(const struct FHitResult& WallHit, FVector& OutCandidate) const;

	// 후보 지점 위에 캐릭터 캡슐이 설 수 있는지 검사
	bool HasStandingRoom(const FVector& Candidate, FVector& OutLandingLocation) const;

	void DrawDebugTarget(const FVector& Start, const FLSRopeTarget& Target) const;

	// 최대 사거리
	UPROPERTY(EditAnywhere, Category = "Rope", meta = (ClampMin = "0.0"))
	float MaxRange = 2000.0f;

	// 스윕 구체 반지름
	UPROPERTY(EditAnywhere, Category = "Rope", meta = (ClampMin = "0.0"))
	float TraceRadius = 15.0f;

	UPROPERTY(EditAnywhere, Category = "Rope")
	TEnumAsByte<ECollisionChannel> RopeTraceChannel = ECC_Visibility;

	// 모서리 탐색: 벽 안쪽으로 들어갈 거리
	UPROPERTY(EditAnywhere, Category = "Rope|Ledge", meta = (ClampMin = "0.0"))
	float LedgeForwardOffset = 30.0f;

	// 모서리 탐색: 명중 지점 위로 탐색할 높이
	UPROPERTY(EditAnywhere, Category = "Rope|Ledge", meta = (ClampMin = "0.0"))
	float LedgeSearchHeight = 150.0f;

	// 착지 캡슐을 바닥에서 띄울 여유
	UPROPERTY(EditAnywhere, Category = "Rope|Landing", meta = (ClampMin = "0.0"))
	float LandingClearance = 5.0f;

	// 스윙 앵커가 캐릭터보다 최소 이만큼 높아야 함
	UPROPERTY(EditAnywhere, Category = "Rope|Swing", meta = (ClampMin = "0.0"))
	float MinSwingHeight = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Rope|Debug")
	bool bDrawDebug = true;

	// 소유 클라가 매 틱 갱신하는 현재 타깃
	FLSRopeTarget CurrentTarget;

private:
	class ACharacter* GetOwnerCharacter() const;

	class ULSCharacterMovementComponent* GetLSMovement() const;
};
