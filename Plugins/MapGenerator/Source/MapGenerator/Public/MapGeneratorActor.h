// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapData.h"
#include "MapGeneratorActor.generated.h"

class UMapGenerationData;

// 던전 생성 제너레이터 액터.
// 런타임: 서버가 BeginPlay에서 시드를 확정·복제 → 서버/모든 클라가 같은 시드로 각자 생성·스트리밍 (결정적 생성이라 시드만 동기화).
// 에디터: Generate 버튼으로 GenerationData->Seed 기반 미리보기 (트랜지언트, 저장/PIE 미반영).
UCLASS()
class MAPGENERATOR_API AMapGenerator : public AActor
{
	GENERATED_BODY()

public:
	AMapGenerator();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// [에디터] 맵 배치 계산 + 방 서브레벨 스트리밍
	UFUNCTION(CallInEditor, Category = "MapGenerator")
	void Generate();

	// [에디터] 스트리밍한 방 인스턴스 모두 언로드
	UFUNCTION(CallInEditor, Category = "MapGenerator")
	void ClearRooms();

protected:
	virtual void BeginPlay() override;

	// 시드 확정 수신 (클라). BeginPlay 이후라면 런타임 생성
	UFUNCTION()
	void OnRep_SeedReady();

	// [런타임] RuntimeSeed로 배치 계산 + 스트리밍 (모든 머신, 1회)
	void GenerateRuntime();

	// true면 서버가 매 판 랜덤 시드 사용, false면 GenerationData->Seed 사용
	UPROPERTY(EditAnywhere, Category = "MapGenerator")
	bool bUseRandomSeed = true;

	// 서버가 확정한 런타임 시드 (클라 복제)
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "MapGenerator")
	int32 RuntimeSeed = 0;

	// 시드 확정 플래그. 시드가 기본값(0)과 같으면 OnRep가 오지 않으므로 bool로 트리거
	UPROPERTY(ReplicatedUsing = OnRep_SeedReady)
	bool bSeedReady = false;

	// 런타임 생성 중복 방지 (비복제 로컬 가드)
	bool bRuntimeGenerated = false;

	// 생성 파라미터 데이터 에셋 (BP에서 편집)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapGenerator", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMapGenerationData> GenerationData;

	// [디버그] 마지막 생성 결과 (인스펙터 확인용)
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "MapGenerator", meta = (AllowPrivateAccess = "true"))
	TArray<FPlacedRoom> LastResult;
};
