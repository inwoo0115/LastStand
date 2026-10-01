// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace LSUITags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Layer_Game);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Layer_Menu);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Layer_Modal);
}

namespace LSAITags
{
	// StateTree 이벤트 (AI 이벤트 허브로 전송)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Initialization);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Idle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Reaction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_AdditionalAction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Death);
}
