// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LSServerSideRewindComponent.generated.h"

class ULSHitboxComponent;
class APawn;
class USkeletalMeshComponent;

// 특정 시점의 히트박스 하나에 대한 월드 공간 스냅샷 (리와인드 시 박스 재구성용)
USTRUCT(BlueprintType)
struct FHitBoxSnapshot
{
	GENERATED_BODY()

	// 월드 중심 위치
	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	// 월드 회전
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	// 박스 반경(스케일 반영)
	UPROPERTY()
	FVector BoxExtent = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FServerSideRewindSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	float Time = 0.0f;

	// 히트박스 버전: 컴포넌트 이름 → 박스 스냅샷
	UPROPERTY()
	TMap<FName, FHitBoxSnapshot> HitBoxSnapshots;

	// 물리 에셋 버전: 바디 월드 트랜스폼 (Mesh->Bodies와 인덱스 1:1)
	UPROPERTY()
	TArray<FTransform> BodyTransforms;
};

// 디버그 형상 종류 (히트박스 + 물리 에셋 바디 공용)
UENUM()
enum class ELSRewindDebugShapeType : uint8
{
	Box     UMETA(DisplayName = "Box"),
	Sphere  UMETA(DisplayName = "Sphere"),
	Capsule UMETA(DisplayName = "Capsule"),
};

// 디버그용 판정 형상 하나 (월드 공간)
USTRUCT()
struct FRewindDebugShape
{
	GENERATED_BODY()

	UPROPERTY()
	ELSRewindDebugShapeType Type = ELSRewindDebugShapeType::Box;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	FQuat Rotation = FQuat::Identity;

	// Box: 반경 / Sphere: X=반지름 / Capsule: X=반지름, Z=절반 높이(반구 포함)
	UPROPERTY()
	FVector Extent = FVector::ZeroVector;

	// 명중한 부위 여부
	UPROPERTY()
	bool bHit = false;
};

// ConfirmHit 1회의 판정 결과 디버그 정보
USTRUCT()
struct FRewindDebugInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FRewindDebugShape> Shapes;

	UPROPERTY()
	FVector TraceStart = FVector::ZeroVector;

	UPROPERTY()
	FVector TraceEnd = FVector::ZeroVector;

	// 실제 되감은 시간(초, 클램프 후)
	UPROPERTY()
	float RewindTime = 0.0f;

	UPROPERTY()
	bool bHit = false;

	// 리와인드 미적용(서버 현재 포즈) 판정 형상 — bDrawDebugNoRewind일 때만 채움
	UPROPERTY()
	TArray<FRewindDebugShape> NoRewindShapes;

	// 리와인드 미적용 판정 결과
	UPROPERTY()
	bool bNoRewindHit = false;

	// 미적용 판정을 수행했는지 (텍스트/그리기 분기용)
	UPROPERTY()
	bool bHasNoRewind = false;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTSTAND_API ULSServerSideRewindComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULSServerSideRewindComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 서버가 측정한 쏜 사람(Shooter)의 RTT만큼 판정 대상을 되돌려 트레이스 명중을 재검증.
	// 명중 시 true. 히트박스 버전은 OutHitbox에 명중 히트박스, 물리 에셋 버전은 nullptr(배율 1.0)
	// OutDebug가 유효하면 되감은 형상/트레이스/결과를 채움 (bDrawDebugConfirm일 때만 호출자가 넘김)
	bool ConfirmHit(const FVector& TraceStart, const FVector& TraceEnd, const APawn* Shooter, ULSHitboxComponent*& OutHitbox, FRewindDebugInfo* OutDebug = nullptr) const;

	bool IsDebugConfirmEnabled() const { return bDrawDebugConfirm; }

	// 서버 판정 결과를 이 머신에 그림 (데디 서버 스킵)
	void DrawConfirmDebug(const FRewindDebugInfo& Info) const;

	// 쏜 클라: 발사 순간 로컬에서 보이는 판정 형상을 파랑으로 그림
	void DrawLocalDebug() const;

	// 서버가 기록한 히트박스 스냅샷을 전 클라에서 디버그 박스로 그림
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDrawDebugBoxes(const TArray<FHitBoxSnapshot>& Boxes);

protected:
	// true: 메쉬의 물리 에셋 바디로 판정(배율 1.0, 히트박스 충돌은 끔) / false: 부위별 히트박스로 판정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind")
	bool bUsePhysicsAsset = false;

	// 물리 에셋 버전에서 메쉬에 적용할 충돌 프로파일 (Hitscan 채널 Block)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind", meta = (EditCondition = "bUsePhysicsAsset"))
	FName BodyCollisionProfileName = TEXT("Enemy");

	// 스냅샷 저장 간격 (20ms)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind")
	float RecordInterval = 0.02f;

	// 버퍼 최대 보관 연령 (200ms) — 이보다 오래된 스냅샷은 제거. 최대 리와인드 연령 겸용(리와인드 0~200ms)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind")
	float HistoryEndOffset = 0.2f;

	// 히트박스 스냅샷 위치를 디버그 박스로 시각화 (히트박스 버전 전용. 물리 에셋 버전은 콘솔 show Collision)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind")
	bool bDrawDebugSnapshot = true;

	// 디버그 박스 전송/표시 주기 (0.5초). 리와인드 저장 주기(RecordInterval, 20ms)와 별개
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind")
	float DebugDrawInterval = 0.5f;

	// 판정 시점(ConfirmHit) 되감은 형상 시각화 (쏜 클라 + 리슨 서버). 파랑=클라 로컬, 노랑=서버 되감음, 빨강=명중 부위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind|Debug")
	bool bDrawDebugConfirm = false;

	// 판정 디버그 표시 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind|Debug", meta = (EditCondition = "bDrawDebugConfirm", ClampMin = "0.0"))
	float DebugConfirmDuration = 3.0f;

	// 같은 트레이스를 서버 현재 포즈(리와인드 0ms)로도 판정해 함께 표시. 마젠타=형상, 주황=명중 부위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind|Debug", meta = (EditCondition = "bDrawDebugConfirm"))
	bool bDrawDebugNoRewind = true;

private:
	// 현재 판정 대상 상태를 스냅샷으로 생성 (Tick 기록 + 미적용 판정 공용)
	FServerSideRewindSnapshot MakeSnapshot(float Time) const;

	// 히트박스 버전: 보간한 히트박스와 세그먼트 교차 검사 → 명중 히트박스 (없으면 nullptr). OutShapes가 유효하면 모든 보간 박스 수집
	ULSHitboxComponent* ConfirmHitboxHit(const FServerSideRewindSnapshot& Before, const FServerSideRewindSnapshot& After, float Alpha, const FVector& TraceStart, const FVector& TraceEnd, TArray<FRewindDebugShape>* OutShapes) const;

	// 물리 에셋 버전: 바디를 과거 포즈로 옮겨 엔진 트레이스 후 복원. OutShapes가 유효하면 되감은 바디 형상 수집
	bool ConfirmBodyHit(const FServerSideRewindSnapshot& Before, const FServerSideRewindSnapshot& After, float Alpha, const FVector& TraceStart, const FVector& TraceEnd, TArray<FRewindDebugShape>* OutShapes) const;

	// 물리 에셋: 바디 트랜스폼 배열(Bodies와 인덱스 1:1) → 바디셋업 AggGeom(Sphere/Box/Sphyl) 형상 수집
	void AppendBodyShapes(const TArray<FTransform>& BodyTMs, int32 HitBodyIndex, TArray<FRewindDebugShape>& OutShapes) const;

	// 히트박스 스냅샷 → 박스 형상
	static FRewindDebugShape MakeBoxShape(const FHitBoxSnapshot& Box, bool bHit);

	// 물리 에셋 버전일 때만 설정되는 판정 메쉬
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> CachedMesh;

	// 히스토리 큐: [0]=가장 오래됨, 마지막=최신
	UPROPERTY()
	TArray<FServerSideRewindSnapshot> History;

	// BeginPlay에서 인터페이스로 수집한 소유 pawn의 히트박스들 (가변 개수)
	UPROPERTY()
	TArray<TObjectPtr<ULSHitboxComponent>> CachedHitboxes;

	// 마지막으로 스냅샷을 저장한 서버 시간
	float LastRecordTime = 0.0f;

	// 마지막으로 디버그 박스를 전송한 서버 시간
	float LastDebugDrawTime = 0.0f;

};
