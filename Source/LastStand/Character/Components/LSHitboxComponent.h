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
 * 부위별 히트박스 컴포넌트.
 * 무기 트레이스(ECC_Visibility)에 맞는 박스 콜리전으로, 어느 부위를 맞았는지(HitboxType)와
 * 해당 부위의 데미지 배율(DamageMultiplier)을 보유한다. 실제 배율 적용은 무기 측에서 수행.
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

protected:
	// 이 히트박스가 대응하는 신체 부위
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	ELSHitboxType HitboxType = ELSHitboxType::Torso;

	// 이 부위에 적중 시 곱해질 데미지 배율 (예: 머리/약점 고배율)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	float DamageMultiplier = 1.0f;
};
