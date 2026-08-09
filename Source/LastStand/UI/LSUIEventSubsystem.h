// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LSUIEventSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInput);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEvent, int32);

// 체력 이벤트 (Current, Max)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHealthEvent, int32, int32);

// 파라미터 없는 HUD 갱신 신호
DECLARE_MULTICAST_DELEGATE(FOnUIRefresh);

// 포커스 무기 탄약 이벤트 (Current, Max)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoEvent, int32, int32);
/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSUIEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 사용자 입력 관련 HUD UI 실행 델리게이트
	FOnInput InventoryInput;

	// 이벤트로 HUD UI 실행 델리게이트
	FOnEvent DamageEvent;

	// 플레이어 체력 HUD 갱신 (Current, Max)
	FOnHealthEvent HealthEvent;

	// 장비 배열 변경(장착/해제/초기화) HUD 갱신
	FOnUIRefresh EquipmentArrayChanged;

	// 포커스 무기 변경 HUD 갱신
	FOnUIRefresh FocusEquipmentChanged;

	// 포커스 무기 탄약 HUD 갱신 (Current, Max)
	FOnAmmoEvent AmmoEvent;

	// 예비 탄약(캐시) 변경 HUD 갱신
	FOnUIRefresh ReserveAmmoChanged;
};
