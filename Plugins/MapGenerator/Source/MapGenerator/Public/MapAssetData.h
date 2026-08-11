// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MapAssetData.generated.h"

class UStaticMesh;   // 소프트 포인터용 전방선언

// WFC 타일 정의. 메쉬/가중치/6면 소켓을 관리 (DataTable 행, RowName이 타일 키).
USTRUCT(BlueprintType)
struct FMapAssetData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // 타일 식별 이름 (행 조회 키(RowName)와 별개 데이터 필드)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
    FName AssetName;

    // 이 타일에 사용할 메쉬
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
    TSoftObjectPtr<UStaticMesh> Mesh;

    // WFC 선택 가중치 (클수록 자주 선택). 0 이하면 후보 제외
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
    float Weight = 1.0f;

    // 6면 소켓 ID (맞닿는 면끼리 호환되면 인접 허용). 호환 판정은 이후 구현
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Sockets")
    FName SocketPosX;   // +X

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Sockets")
    FName SocketNegX;   // -X

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Sockets")
    FName SocketPosY;   // +Y

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Sockets")
    FName SocketNegY;   // -Y

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Sockets")
    FName SocketPosZ;   // +Z (위)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile|Sockets")
    FName SocketNegZ;   // -Z (아래)
};