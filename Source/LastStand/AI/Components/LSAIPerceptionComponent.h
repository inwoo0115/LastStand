// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "LSAIPerceptionComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LASTSTAND_API ULSAIPerceptionComponent : public UCapsuleComponent
{
	GENERATED_BODY()

public:
	ULSAIPerceptionComponent();

	AActor* GetTargetActor() const { return TargetActor; }

	// 현재 가장 가까운 타깃 (서버 전용, 복제 안 함)
	UPROPERTY()
	TObjectPtr<AActor> TargetActor = nullptr;
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 후보 중 가장 가까운 액터로 TargetActor 갱신 (변경 시 델리게이트 브로드캐스트)
	void UpdateClosestTarget();

	// 감지 볼륨 크기 (에디터 조정)
	UPROPERTY(EditAnywhere, Category = "Perception")
	float DetectionRadius = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Perception")
	float DetectionHalfHeight = 200.0f;

	// 캡슐 안의 ALSCharacterBase 후보들 (서버 전용)
	UPROPERTY()
	TArray<TObjectPtr<AActor>> PerceivedActors;

	
};
