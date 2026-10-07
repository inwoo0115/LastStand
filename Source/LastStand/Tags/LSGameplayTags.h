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

	// 서브트리 슬롯 (Base 트리의 Linked Asset 상태 Tag — EnemyData로 교체)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subtree);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subtree_Idle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subtree_Initialization);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subtree_Combat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subtree_Reaction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subtree_AdditionalAction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Subtree_Death);

	// 행동 (AI Action 컴포넌트가 실행 — EnemyData의 Actions 키)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Attack_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Attack_Ranged);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Initialization);
}
