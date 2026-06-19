// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LSUISubsystem.h"
#include "LSSlotWidget.h"
#include "LSRootLayoutWidget.h"
#include "LSLayerWidget.h"
#include "Tags/LSGameplayTags.h"
#include "Framework/Application/SlateApplication.h"

void ULSUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void ULSUISubsystem::InitializeRootLayout(APlayerController* PC, TSubclassOf<class ULSRootLayoutWidget> LayoutClass)
{
	if (!PC || !LayoutClass || !PC->IsLocalController()) return;

	RootLayout = CreateWidget<ULSRootLayoutWidget>(PC, LayoutClass);

	if (!RootLayout) return;

	RootLayout->AddToPlayerScreen(1000);

	
}

void ULSUISubsystem::RegisterLayer(FGameplayTag LayerTag, ULSLayerWidget* LayerWidget)
{
	if (!LayerTag.IsValid() || !LayerWidget) return;
	LayerMap.Add(LayerTag, LayerWidget);
}

UUserWidget* ULSUISubsystem::PushWidgetToLayer(FGameplayTag LayerTag, TSubclassOf<UUserWidget> WidgetClass)
{
	if (ULSLayerWidget* Layer = LayerMap.FindRef(LayerTag))
	{
		return Layer->PushWidget(WidgetClass);
	}
	return nullptr;
}

void ULSUISubsystem::PopWidgetFromLayer(UUserWidget* Widget)
{
	if (!Widget) return;

	// 위젯 제거
	for (auto& [LayerTag, LayerWidget] : LayerMap)
	{
		if (LayerWidget && LayerWidget->ContainsWidget(Widget))
		{
			LayerWidget->PopWidget(Widget);
			return;
		}
	}
}

void ULSUISubsystem::RegisterSlot(FGameplayTag SlotTag, ULSSlotWidget* SlotWidget)
{
	SlotWidgetMap.FindOrAdd(SlotTag).Add(SlotWidget);

	if (TArray<FSlotEntry>* Pending = PendingInjections.Find(SlotTag))
	{
		for (const FSlotEntry& Entry : *Pending)
		{
			SlotWidget->AddWidget(Entry.WidgetClass, Entry.HandleId);
		}
		PendingInjections.Remove(SlotTag);
	}
}

void ULSUISubsystem::UnregisterSlot(FGameplayTag SlotTag, ULSSlotWidget* SlotWidget)
{
	if (TArray<TWeakObjectPtr<ULSSlotWidget>>* Slots = SlotWidgetMap.Find(SlotTag))
	{
		Slots->RemoveAll([SlotWidget](const TWeakObjectPtr<ULSSlotWidget>& Slot)
			{
				return !Slot.IsValid() || Slot.Get() == SlotWidget;
			});


		if (Slots->IsEmpty())
		{
			SlotWidgetMap.Remove(SlotTag);
		}
	}
}

FSlotHandle ULSUISubsystem::InjectWidgetToSlot(FGameplayTag SlotTag, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!SlotTag.IsValid() || !WidgetClass) return FSlotHandle{};
	const int32 HandleId = NextHandleId++;
	FSlotHandle Handle{ SlotTag, HandleId };

	if (TArray<TWeakObjectPtr<ULSSlotWidget>>* Slots = SlotWidgetMap.Find(SlotTag))
	{
		for (TWeakObjectPtr<ULSSlotWidget>& Slot : *Slots)
		{
			if (Slot.IsValid())
			{
				Slot->AddWidget(WidgetClass, HandleId);
			}
		}
	}
	else
	{
		PendingInjections.FindOrAdd(SlotTag).Add({ WidgetClass, HandleId });
	}

	return Handle;
}

void ULSUISubsystem::RemoveWidgetFromSlot(FSlotHandle Handle)
{
	if (TArray<TWeakObjectPtr<ULSSlotWidget>>* Slots = SlotWidgetMap.Find(Handle.SlotTag))
	{
		for (TWeakObjectPtr<ULSSlotWidget>& Slot : *Slots)
		{
			if (Slot.IsValid())
			{
				Slot->RemoveWidget(Handle.HandleId);
			}
		}
	}
	if (TArray<FSlotEntry>* Pending = PendingInjections.Find(Handle.SlotTag))
	{
		Pending->RemoveAll([&](const FSlotEntry& E) { return E.HandleId == Handle.HandleId; });
	}
}

bool ULSUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) return false;
	// 데디케이티드 서버에서는 UI 서브시스템 불필요
	return !IsRunningDedicatedServer();
}

void ULSUISubsystem::UpdateInputType()
{
	// UI 우선순위 대로 입력 처리 설정
	if (SetInputTypeByTag(LSUITags::Layer_Menu))
	{
		return;
	}
	else if (SetInputTypeByTag(LSUITags::Layer_Modal))
	{
		return;
	}
	else if (SetInputTypeByTag(LSUITags::Layer_Game))
	{
		return;
	}
}

bool ULSUISubsystem::SetInputTypeByTag(FGameplayTag LayerTag)
{
	if (TObjectPtr<ULSLayerWidget>* LayerPtr = LayerMap.Find(LayerTag))
	{
		ULSLayerWidget* Layer = *LayerPtr;
		if (Layer->GetIsActivated())
		{
			// 최상단에 입력 고정
			Layer->FocusOnTopWidget();

			// Layer 기반 인풋 변경
			EInputType NewInputType = Layer->GetInputType();
			UpdateInputMode(NewInputType);
			return true;
		}
	}
	return false;
}

void ULSUISubsystem::UpdateInputMode(EInputType NewType)
{
	if (InputType == NewType)
	{
		return;
	}

	const UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	const ULocalPlayer* LocalPlayer = GI->GetFirstGamePlayer();
	if (!LocalPlayer) return;

	APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
	if (!PC) return;

	InputType = NewType;

	switch (InputType)
	{
	case EInputType::Game:
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
	break;

	case EInputType::GameAndUI:
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);

		if (GEngine && GEngine->GameViewport)
		{
			FSlateApplication::Get().SetAllUserFocusToGameViewport();
		}
	}
	break;

	case EInputType::UIOnly:
	{
		FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);

		if (GEngine && GEngine->GameViewport)
		{
			FSlateApplication::Get().SetAllUserFocusToGameViewport();
		}
	}
	break;
	}
}
