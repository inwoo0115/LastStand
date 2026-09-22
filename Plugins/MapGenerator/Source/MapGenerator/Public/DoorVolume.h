// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoorVolume.generated.h"

class UBoxComponent;
class UArrowComponent;

// 방 서브레벨에 배치해 문 위치/방향/입구여부를 표시하는 디자인 마커.
// 런타임 콜리전은 없고, 에디터에서 이 볼륨들을 FMapData의 EntranceDoor/ExitDoors로 export한다.
// 좌표는 GetActorLocation(), 방향은 액터 yaw를 4방위로 스냅해서 export 측에서 취득.
UCLASS()
class MAPGENERATOR_API ADoorVolume : public AActor
{
	GENERATED_BODY()

public:
	ADoorVolume();

	// true=입구 문(방마다 정확히 1개), false=출구 문
	bool IsEntrance() const { return bIsEntrance; }

protected:
	// 문 마커 박스 (루트). 에디터에서 위치/회전 조정 — +X가 문이 향하는 방향(North 기준)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> BoxComponent;

	// 문이 향하는 방향 시각화 (+X 전방)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArrowComponent> ArrowComponent;

	// 입구/출구 마킹. true=입구, false=출구
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivateAccess = "true"))
	bool bIsEntrance = false;
};
