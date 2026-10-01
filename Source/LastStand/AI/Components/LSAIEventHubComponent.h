// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "StructUtils/StructView.h"
#include "StructUtils/InstancedStruct.h"
#include "LSAIEventHubComponent.generated.h"

// 게임플레이 → StateTree 이벤트 전달 통로를 일원화하는 허브 (서버 전용, 비복제)
// 발신원은 이 컴포넌트에 태그 + 페이로드로 SendEvent를 호출하고, StateTree 전송은 허브 내부에서만 수행
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LASTSTAND_API ULSAIEventHubComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULSAIEventHubComponent();

	// C++ 발신용: 태그 + 페이로드(구조체 뷰)로 StateTree에 이벤트 전송 (서버 전용)
	void SendEvent(FGameplayTag EventTag, FConstStructView Payload = FConstStructView(), FName Origin = NAME_None);

	// BP/AnimNotify 발신용 래퍼
	UFUNCTION(BlueprintCallable, Category = "AI|Event", meta = (DisplayName = "Send AI Event"))
	void K2_SendEvent(FGameplayTag EventTag, const FInstancedStruct& Payload, FName Origin);

protected:
	// 소유 폰의 AI 컨트롤러에서 StateTree 컴포넌트 조회 (지연 해석 + 약참조 캐시)
	class UStateTreeComponent* ResolveStateTreeComponent();

	// 캐시된 StateTree 컴포넌트 (possess 변경/파괴 대비 약참조)
	TWeakObjectPtr<class UStateTreeComponent> CachedStateTreeComp;
};
