// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "LSAIActionComponentInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULSAIActionComponentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class LASTSTAND_API ILSAIActionComponentInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 행동(공격 등) 실행기인 AI 액션 컴포넌트 반환
	virtual class ULSAIActionComponent* GetAIActionComponent() = 0;
};
