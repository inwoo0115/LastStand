// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "LSHitboxComponent.generated.h"

// 히트박스가 대응하는 신체 부위
UENUM(BlueprintType)
enum class ELSHitboxType : uint8
{
	Head		UMETA(DisplayName = "Head"),
	Torso		UMETA(DisplayName = "Torso"),
	LeftArm		UMETA(DisplayName = "Left Arm"),
	RightArm	UMETA(DisplayName = "Right Arm"),
	LeftLeg		UMETA(DisplayName = "Left Leg"),
	RightLeg	UMETA(DisplayName = "Right Leg"),
	WeakPoint	UMETA(DisplayName = "Weak Point"),
};

/**
 * 부위별 히트박스 컴포넌트. 적 BP에서 추가하고 메쉬의 Parent Socket에 부착해 사용한다.
 * Hitscan 채널 트레이스에 맞는 박스 콜리전으로, 부위(HitboxType)와 데미지 배율(DamageMultiplier)을 보유한다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LASTSTAND_API ULSHitboxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	ULSHitboxComponent();

	ELSHitboxType GetHitboxType() const { return HitboxType; }
	float GetDamageMultiplier() const { return DamageMultiplier; }

	// 소유 액터 생성자에서 부위/배율을 주입 (멤버가 protected라 외부 클래스가 직접 설정 불가)
	void SetupHitbox(ELSHitboxType InType, float InMultiplier);

	// 로컬 예측: RawDamage에 부위 배율을 곱해 owner StatComponent의 CalculateDamage로 전달
	void ProcessLocalHit(int32 RawDamage);

	// 서버 권위: RawDamage에 부위 배율을 곱해 owner StatComponent의 ApplyDamage로 전달
	// DamageCauser: 데미지를 넣은 액터 (ApplyDamage로 그대로 전달)
	void ProcessServerHit(int32 RawDamage, AActor* DamageCauser);

protected:
	// 이 히트박스가 대응하는 신체 부위
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	ELSHitboxType HitboxType = ELSHitboxType::Torso;

	// 이 부위에 적중 시 곱해질 데미지 배율 (예: 머리/약점 고배율)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	float DamageMultiplier = 1.0f;
};
