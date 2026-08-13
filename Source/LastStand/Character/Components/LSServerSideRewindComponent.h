// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LSServerSideRewindComponent.generated.h"

class ULSHitboxComponent;

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

	UPROPERTY()
	TMap<FName, FHitBoxSnapshot> HitBoxSnapshots;
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

	// 서버에서 발사 시점(Timestamp)으로 히트박스를 되돌려 트레이스 명중을 재검증.
	// 명중한 히트박스를 반환, 스냅샷 없음/판정 실패/미명중이면 nullptr.
	class ULSHitboxComponent* ConfirmHit(const FVector& TraceStart, const FVector& TraceEnd, float Timestamp) const;

	// 서버가 기록한 히트박스 스냅샷을 전 클라에서 디버그 박스로 그림
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDrawDebugBoxes(const TArray<FHitBoxSnapshot>& Boxes);

protected:
	// 스냅샷 저장 간격 (20ms)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind")
	float RecordInterval = 0.02f;

	// 버퍼 최대 보관 연령 (200ms) — 이보다 오래된 스냅샷은 제거. 최대 리와인드 연령 겸용(리와인드 0~200ms)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind")
	float HistoryEndOffset = 0.2f;

	// 스냅샷 위치를 디버그 박스로 시각화
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind")
	bool bDrawDebugSnapshot = true;

	// 디버그 박스 전송/표시 주기 (0.5초). 리와인드 저장 주기(RecordInterval, 20ms)와 별개
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Server Side Rewind")
	float DebugDrawInterval = 0.5f;

private:
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

	// TODO(후속): ConfirmHit이 명중 히트박스(부위/DamageMultiplier)를 out으로 반환 → 무기에서 배율 데미지 적용
};
