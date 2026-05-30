// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "LSUISubsystem.generated.h"

USTRUCT(BlueprintType)
struct FSlotHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag SlotTag;

	UPROPERTY(BlueprintReadOnly)
	int32 HandleId = INDEX_NONE;
};

USTRUCT()
struct FSlotEntry
{
	GENERATED_BODY()
	TSubclassOf<UUserWidget> WidgetClass;
	int32 HandleId = INDEX_NONE;
};


/**
 * 
 */
UCLASS()
class LASTSTAND_API ULSUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeRootLayout(APlayerController* PC, TSubclassOf<class ULSRootLayoutWidget> LayoutClass);

	// Layer API
	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	void RegisterLayer(FGameplayTag LayerTag, class ULSLayerWidget* LayerWidget);

	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	UUserWidget* PushWidgetToLayer(FGameplayTag LayerTag, TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "UI|Layer")
	void PopWidgetFromLayer(UUserWidget* Widget);

	// Slot API
	void RegisterSlot(FGameplayTag SlotTag, class ULSSlotWidget* SlotWidget);
	void UnregisterSlot(FGameplayTag SlotTag, class ULSSlotWidget* SlotWidget);

	UFUNCTION(BlueprintCallable, Category = "UI|Slot")
	FSlotHandle InjectWidgetToSlot(FGameplayTag SlotTag, TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "UI|Slot")
	void RemoveWidgetFromSlot(FSlotHandle Handle);

protected:
	UPROPERTY()
	TObjectPtr<ULSRootLayoutWidget> RootLayout;

	// Layers
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<class ULSLayerWidget>> LayerMap;

	// 위젯Slot
	TMap<FGameplayTag, TArray<TWeakObjectPtr<class ULSSlotWidget>>> SlotWidgetMap;

	// 슬롯 주입 대기
	TMap<FGameplayTag, TArray<FSlotEntry>> PendingInjections;

	int32 NextHandleId = 0;
	
	
};
