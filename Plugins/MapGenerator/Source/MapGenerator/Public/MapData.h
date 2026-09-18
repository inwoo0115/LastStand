// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MapData.generated.h"

class UWorld;   // 소프트 포인터용 전방선언

// 문이 향하는 방향 (방-방 연결 매칭용 4방위)
UENUM(BlueprintType)
enum class EDoorDirection : uint8
{
	North UMETA(DisplayName = "North"),
	East  UMETA(DisplayName = "East"),
	South UMETA(DisplayName = "South"),
	West  UMETA(DisplayName = "West")
};

// 방 형태
UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Start    UMETA(DisplayName = "Start"),     // 시작 지점
	End      UMETA(DisplayName = "End"),       // 도착 지점
	Normal   UMETA(DisplayName = "Normal"),    // 일반 방
	Corridor UMETA(DisplayName = "Corridor")   // 통로
};

// 방의 문 하나 (좌표 + 방향)
USTRUCT(BlueprintType)
struct FRoomDoor
{
	GENERATED_BODY()

	// 방 원점 기준 로컬 문 위치(cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	FVector Location = FVector::ZeroVector;

	// 문이 향하는 방향 (연결 매칭용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	EDoorDirection Direction = EDoorDirection::North;
};

// 던전 방 한 칸의 정보. 데이터 테이블 행(RowName = 방 식별자).
USTRUCT(BlueprintType)
struct FMapData : public FTableRowBase
{
	GENERATED_BODY()

	// 방 서브레벨 (스트리밍 로드 대상).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	TSoftObjectPtr<UWorld> RoomLevel;

	// 방 형태 (시작/도착/일반/통로)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	ERoomType RoomType = ERoomType::Normal;

	// 문 개수 (명시 — Doors 배열 길이와 일치하도록 관리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	int32 DoorCount = 0;

	// 문 목록 (좌표 + 방향)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	TArray<FRoomDoor> Doors;
};
