// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoxVolume.generated.h"

class UBoxComponent;

// 방 서브레벨에 배치해 방 경계를 표시하는 디자인 마커.
// 런타임 콜리전은 없고, 에디터에서 이 볼륨들의 바운즈를 FMapData로 export한다.
UCLASS()
class MAPGENERATOR_API ABoxVolume : public AActor
{
	GENERATED_BODY()

public:
	ABoxVolume();

	UBoxComponent* GetBoxComponent() const { return BoxComponent; }

protected:
	// 방 경계 박스 (루트). 에디터에서 크기 조정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> BoxComponent;
};
