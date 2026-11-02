// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "LSEnvQueryContext_Target.generated.h"

/**
 * EQS 컨텍스트: 쿼리 주인(적)의 ULSAIPerceptionComponent가 선정한 현재 타깃 (서버 전용)
 */
UCLASS(meta = (DisplayName = "LS Target"))
class LASTSTAND_API ULSEnvQueryContext_Target : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
