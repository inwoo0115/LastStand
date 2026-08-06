// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Interface/LSStatComponentInterface.h"
#include "LSEnemyBase.generated.h"

UCLASS()
class LASTSTAND_API ALSEnemyBase : public APawn, public ILSStatComponentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ALSEnemyBase();

	virtual class ULSStatComponent* GetStatComponent() override;

	virtual void ApplyDamage(int32 Damage) override;

	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 루트 캡슐 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCapsuleComponent> Capsule;

	// 스탯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULSStatComponent> Stat;

	// 스탯 초기화에 사용할 데이터 테이블(FEnemyData) 행 이름
	UPROPERTY(EditAnywhere, Category = Stat, meta = (AllowPrivateAccess = "true"))
	FName EnemyName;

	// 체력바 위젯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UWidgetComponent> HealthBarWidget;
};
