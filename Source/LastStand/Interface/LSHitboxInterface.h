// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "LSHitboxInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULSHitboxInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 부위별 히트박스(ULSHitboxComponent)를 보유한 액터가 구현하는 인터페이스.
 */
class LASTSTAND_API ILSHitboxInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 이 액터가 보유한 모든 히트박스를 반환 — 실제 구현은 액터가 담당
	virtual void GetHitboxComponents(TArray<class ULSHitboxComponent*>& OutHitboxes) const = 0;
};
