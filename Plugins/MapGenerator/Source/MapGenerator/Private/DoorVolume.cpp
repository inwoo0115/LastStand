// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorVolume.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

ADoorVolume::ADoorVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = BoxComponent;

	// 순수 마커 — export된 좌표/방향으로 판정하므로 런타임 콜리전 불필요
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoxComponent->SetBoxExtent(FVector(100.0f, 100.0f, 150.0f));

	// +X가 문이 향하는 방향 (액터 yaw → 4방위 스냅)
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(BoxComponent);
}
