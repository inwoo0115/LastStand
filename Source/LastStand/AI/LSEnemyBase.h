// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Interface/LSStatComponentInterface.h"
#include "Interface/LSHitboxInterface.h"
#include "LSEnemyBase.generated.h"

UCLASS()
class LASTSTAND_API ALSEnemyBase : public APawn, public ILSStatComponentInterface, public ILSHitboxInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ALSEnemyBase();

	virtual class ULSStatComponent* GetStatComponent() override;

	virtual void ApplyDamage(int32 Damage) override;

	// 이 적이 보유한 모든 부위 히트박스를 반환
	virtual void GetHitboxComponents(TArray<class ULSHitboxComponent*>& OutHitboxes) const override;

	virtual void Tick(float DeltaTime) override;

	class UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 루트 캡슐 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCapsuleComponent> Capsule;

	// 스켈레탈 메시 (애셋/애님 클래스는 BP에서 지정). 히트박스 소켓 부착 대상
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh;

	// 스탯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSStatComponent> Stat;

	// 서버 사이드 리와인드(히트박스 히스토리 기록) 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSServerSideRewindComponent> ServerSideRewind;

	// 스탯 초기화에 사용할 데이터 테이블(FEnemyData) 행 이름
	UPROPERTY(EditAnywhere, Category = Stat, meta = (AllowPrivateAccess = "true"))
	FName EnemyName;

	// 체력바 위젯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UWidgetComponent> HealthBarWidget;

	// --- 부위별 히트박스 (본 소켓 재부착·크기 조정은 BP에서 마무리) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> HeadHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> TorsoHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> LeftArmHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> RightArmHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> LeftLegHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> RightLegHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hitbox, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSHitboxComponent> WeakPointHitbox;

	// 이 적이 실행할 비헤이비어 트리 (적 BP에서 지정)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBehaviorTree> BehaviorTree;
};
