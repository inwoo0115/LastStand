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
	if (!FRootMotionSource::Matches(Other))
	{
		return false;
	}

	// 베이스에서 ScriptStruct 일치 확인 → 안전한 캐스트
	// StartLocation은 머신별 현재 위치라 비교에서 제외 (Target/ArcHeight는 클라가 확정해 전송한 같은 값)
	const FLSRootMotionSource_ParabolicMoveTo* OtherCast = static_cast<const FLSRootMotionSource_ParabolicMoveTo*>(Other);
	return FVector::PointsAreNear(TargetLocation, OtherCast->TargetLocation, 0.1f) &&
		FMath::IsNearlyEqual(ArcHeight, OtherCast->ArcHeight, 0.1f);
}

bool FLSRootMotionSource_ParabolicMoveTo::MatchesAndHasSameState(const FRootMotionSource* Other) const
{
	// 시간 외 고유 상태 없음
	return FRootMotionSource::MatchesAndHasSameState(Other);
}

bool FLSRootMotionSource_ParabolicMoveTo::UpdateStateFrom(const FRootMotionSource* SourceToTakeStateFrom, bool bMarkForSimulatedCatchup)
{
	return FRootMotionSource::UpdateStateFrom(SourceToTakeStateFrom, bMarkForSimulatedCatchup);
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
