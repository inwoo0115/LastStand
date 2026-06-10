// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/Components/LSGameModeInfoComponent.h"
#include "LSGameModeInfoComponent.h"
#include "Net/UnrealNetwork.h"
#include "Data/LSGameModeInfoData.h"
#include "UI/LSUISubsystem.h"
#include "Tags/LSGameplayTags.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"

ULSGameModeInfoComponent::ULSGameModeInfoComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void ULSGameModeInfoComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner()->HasAuthority())
	{
		OnInfoDataRep();
	}
}

void ULSGameModeInfoComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULSGameModeInfoComponent, InfoData);
}

void ULSGameModeInfoComponent::OnInfoDataRep()
{
	// 리플리케이션 되고 클라이언트에서 해당 게임 모드 설정 적용
	if (!InfoData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LSGameModeInfoComponent] OnInfoDataRep: InfoData is null."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LSGameModeInfoComponent] OnInfoDataRep: World is null."));
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LSGameModeInfoComponent] OnInfoDataRep: GameInstance is null."));
		return;
	}

	ULSUISubsystem* UISubsystem = GI->GetSubsystem<ULSUISubsystem>();
	if (!UISubsystem)
	{
		// 데디케이티드 서버 등 UI 서브시스템이 없는 환경
		UE_LOG(LogTemp, Warning, TEXT("[LSGameModeInfoComponent] OnInfoDataRep: LSUISubsystem not found."));
		return;
	}

	PushWidgetsToLayer(UISubsystem, InfoData->GameWidgets,  LSUITags::Layer_Game);
	PushWidgetsToLayer(UISubsystem, InfoData->MenuWidgets,  LSUITags::Layer_Menu);
	PushWidgetsToLayer(UISubsystem, InfoData->ModalWidgets, LSUITags::Layer_Modal);
}

void ULSGameModeInfoComponent::PushWidgetsToLayer(ULSUISubsystem* UISubsystem, const TArray<TSoftClassPtr<UUserWidget>>& Widgets, FGameplayTag LayerTag)
{
	for (const TSoftClassPtr<UUserWidget>& SoftClass : Widgets)
	{
		if (SoftClass.IsNull())
		{
			continue;
		}

		UClass* WidgetClass = SoftClass.LoadSynchronous();
		if (!WidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LSGameModeInfoComponent] Failed to load widget class: %s"),
				*SoftClass.ToString());
			continue;
		}

		UISubsystem->PushWidgetToLayer(LayerTag, WidgetClass);
	}
}
