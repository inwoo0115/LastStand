// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "LSGameState.generated.h"

/**
 * 
 */
UCLASS()
class LASTSTAND_API ALSGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	ALSGameState();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// InfoComp에 설정된 컨트롤 데이터 반환 (없으면 nullptr)
	class ULSCharacterControlData* GetControlData() const;

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class ULSGameModeInfoComponent> InfoComp;

	// 던전 배치 리플리케이션 + 서브레벨 스트리밍 (MapGenerator 플러그인)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UMapGeneratorComponent> MapGenComp;

};
