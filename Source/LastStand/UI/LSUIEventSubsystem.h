// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LSUIEventSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInput);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEvent, int32);
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
};
