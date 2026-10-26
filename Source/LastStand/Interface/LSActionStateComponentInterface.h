// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "LSActionStateComponentInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULSActionStateComponentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class LASTSTAND_API ILSActionStateComponentInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 액션 차단/취소 판정 컴포넌트 (무기 등 외부에서 소유자 상태 확인용)
	virtual class ULSActionStateComponent* GetActionStateComponent() = 0;
};
