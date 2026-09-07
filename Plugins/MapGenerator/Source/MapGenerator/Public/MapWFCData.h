// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MapWFCData.generated.h"

class UStaticMesh;   // 소프트 포인터용 전방선언

// WFC 전용 파라미터. 데이터 테이블 행으로 관리 (FMapData와 동일 RowName으로 매칭).
// WFC는 런타임 루프에서 제외된 dormant 기능 — 이 테이블은 지연 로드된다.
USTRUCT(BlueprintType)
struct FMapWFCData : public FTableRowBase
{
	GENERATED_BODY()

public:
	FMapWFCData();

	// 맵 식별 이름 (FMapData::MapName과 동일 RowName으로 매칭되는 데이터 필드)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	FName MapName;

	// 타일 한 칸의 크기(cm). X=가로, Y=세로, Z=높이(층 간격). WFC 3D 시각화용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	FVector CellSize = FVector(100.0f);

	// WFC 결정성 시드 (붕괴 선택에 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	int32 WFCSeed = 0;

	// WFC 경계 소켓: 그리드 가장자리/공중 면이 호환돼야 하는 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	FName EmptySocket = TEXT("Empty");

	// WFC 바닥 소켓: z=0 셀의 -Z 면이 호환돼야 하는 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	FName FloorSocket = TEXT("Floor");

	// [디버그] WFC 타일 시각화에 인스턴싱할 폴백 메쉬 (기본: 엔진 큐브)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	TSoftObjectPtr<UStaticMesh> DebugMesh;
};
