// Fill out your copyright notice in the Description page of Project Settings.


#include "MapData.h"

FMapData::FMapData()
{
	// [임시/디버그] 기본 시각화 메쉬를 엔진 기본 큐브로 설정
	DebugMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube")));
}

