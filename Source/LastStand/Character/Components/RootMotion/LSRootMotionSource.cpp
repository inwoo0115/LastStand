// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/RootMotion/LSRootMotionSource.h"
#include "GameFramework/Character.h"

FLSRootMotionSource_ParabolicMoveTo::FLSRootMotionSource_ParabolicMoveTo()
{
}

FVector FLSRootMotionSource_ParabolicMoveTo::GetLocationAtFraction(float MoveFraction) const
{
	const float T = FMath::Clamp(MoveFraction, 0.0f, 1.0f);

	// 직선 보간 + 위로 솟는 포물선 (t=0.5에서 ArcHeight)
	return FMath::Lerp(StartLocation, TargetLocation, T) + FVector::UpVector * (4.0f * ArcHeight * T * (1.0f - T));
}

FRootMotionSource* FLSRootMotionSource_ParabolicMoveTo::Clone() const
{
	return new FLSRootMotionSource_ParabolicMoveTo(*this);
}

bool FLSRootMotionSource_ParabolicMoveTo::Matches(const FRootMotionSource* Other) const
{
	return FRootMotionSource::Matches(Other);
}

bool FLSRootMotionSource_ParabolicMoveTo::MatchesAndHasSameState(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::MatchesAndHasSameState(Other))
	{
		return false;
	}

	// 궤적(서버 확정값)이 다르면 다른 상태 → 클라가 재시뮬레이션하며 UpdateStateFrom으로 서버 궤적을 받음
	// 허용 오차 1cm: 직렬화 오차는 같은 상태로 취급
	const FLSRootMotionSource_ParabolicMoveTo* OtherCast = static_cast<const FLSRootMotionSource_ParabolicMoveTo*>(Other);
	return FVector::PointsAreNear(StartLocation, OtherCast->StartLocation, 1.0f)
		&& FVector::PointsAreNear(TargetLocation, OtherCast->TargetLocation, 1.0f)
		&& FMath::IsNearlyEqual(ArcHeight, OtherCast->ArcHeight, 1.0f);
}

bool FLSRootMotionSource_ParabolicMoveTo::UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom, bool bMarkForSimulatedCatchup)
{
	if (!FRootMotionSource::UpdateStateFrom(SourceToTakeStateFrom, bMarkForSimulatedCatchup))
	{
		return false;
	}

	// 서버가 확정한 궤적으로 교체 → 남은 구간이 서버 착지점으로 수렴 (베이스에서 ScriptStruct 일치 확인됨)
	const FLSRootMotionSource_ParabolicMoveTo* Other = static_cast<const FLSRootMotionSource_ParabolicMoveTo*>(SourceToTakeStateFrom);
	StartLocation = Other->StartLocation;
	TargetLocation = Other->TargetLocation;
	ArcHeight = Other->ArcHeight;
	return true;
}

void FLSRootMotionSource_ParabolicMoveTo::PrepareRootMotion(float SimulationTime, float MovementTickTime, const ACharacter& Character, const UCharacterMovementComponent& MoveComponent)
{
	RootMotionParams.Clear();

	if (Duration > UE_SMALL_NUMBER && MovementTickTime > UE_SMALL_NUMBER)
	{
		// 이번 틱 끝에 있어야 할 위치로 가는 속도 (MoveToForce와 동일 방식 → 오차 자가 보정)
		const float MoveFraction = (GetTime() + SimulationTime) / Duration;
		const FVector Force = (GetLocationAtFraction(MoveFraction) - Character.GetActorLocation()) / MovementTickTime;
		RootMotionParams.Set(FTransform(Force));
	}

	SetTime(GetTime() + SimulationTime);
}

bool FLSRootMotionSource_ParabolicMoveTo::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	Ar << StartLocation;
	Ar << TargetLocation;
	Ar << ArcHeight;

	bOutSuccess = true;
	return true;
}

UScriptStruct* FLSRootMotionSource_ParabolicMoveTo::GetScriptStruct() const
{
	return FLSRootMotionSource_ParabolicMoveTo::StaticStruct();
}

FString FLSRootMotionSource_ParabolicMoveTo::ToSimpleString() const
{
	return FString::Printf(TEXT("[ID:%u]FLSRootMotionSource_ParabolicMoveTo %s"), LocalID, *InstanceName.GetPlainNameString());
}
