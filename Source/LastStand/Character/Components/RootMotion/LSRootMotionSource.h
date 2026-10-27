// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/RootMotionSource.h"
#include "LSRootMotionSource.generated.h"

// 시작점 → 목표점을 포물선으로 이동 (직선 위로 ArcHeight만큼 솟는 궤적, 높이 차 있는 착지 지원)
USTRUCT()
struct LASTSTAND_API FLSRootMotionSource_ParabolicMoveTo : public FRootMotionSource
{
	GENERATED_BODY()

	FLSRootMotionSource_ParabolicMoveTo();

	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	// 시작-목표 직선 기준 포물선 최고 높이
	UPROPERTY()
	float ArcHeight = 0.0f;

	// 진행률(0~1) 위치
	FVector GetLocationAtFraction(float MoveFraction) const;

	virtual FRootMotionSource* Clone() const override;
	virtual bool Matches(const FRootMotionSource* Other) const override;
	virtual bool MatchesAndHasSameState(const FRootMotionSource* Other) const override;
	virtual bool UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom, bool bMarkForSimulatedCatchup = false) override;
	virtual void PrepareRootMotion(float SimulationTime, float MovementTickTime, const ACharacter& Character, const UCharacterMovementComponent& MoveComponent) override;
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual FString ToSimpleString() const override;
};

template<>
struct TStructOpsTypeTraits<FLSRootMotionSource_ParabolicMoveTo> : public TStructOpsTypeTraitsBase2<FLSRootMotionSource_ParabolicMoveTo>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
