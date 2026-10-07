// Fill out your copyright notice in the Description page of Project Settings.


#include "Tags/LSGameplayTags.h"

namespace LSUITags
{
	UE_DEFINE_GAMEPLAY_TAG(Layer_Game, "UI.Game");
	UE_DEFINE_GAMEPLAY_TAG(Layer_Menu, "UI.Menu");
	UE_DEFINE_GAMEPLAY_TAG(Layer_Modal, "UI.Modal");
}

namespace LSAITags
{
	UE_DEFINE_GAMEPLAY_TAG(Event, "AI.Event");
	UE_DEFINE_GAMEPLAY_TAG(Event_Initialization, "AI.Event.Initialization");
	UE_DEFINE_GAMEPLAY_TAG(Event_Idle, "AI.Event.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat, "AI.Event.Combat");
	UE_DEFINE_GAMEPLAY_TAG(Event_Reaction, "AI.Event.Reaction");
	UE_DEFINE_GAMEPLAY_TAG(Event_AdditionalAction, "AI.Event.AdditionalAction");
	UE_DEFINE_GAMEPLAY_TAG(Event_Death, "AI.Event.Death");

	UE_DEFINE_GAMEPLAY_TAG(Subtree, "AI.Subtree");
	UE_DEFINE_GAMEPLAY_TAG(Subtree_Idle, "AI.Subtree.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Subtree_Initialization, "AI.Subtree.Initialization");
	UE_DEFINE_GAMEPLAY_TAG(Subtree_Combat, "AI.Subtree.Combat");
	UE_DEFINE_GAMEPLAY_TAG(Subtree_Reaction, "AI.Subtree.Reaction");
	UE_DEFINE_GAMEPLAY_TAG(Subtree_AdditionalAction, "AI.Subtree.AdditionalAction");
	UE_DEFINE_GAMEPLAY_TAG(Subtree_Death, "AI.Subtree.Death");

	UE_DEFINE_GAMEPLAY_TAG(Action, "AI.Action");
	UE_DEFINE_GAMEPLAY_TAG(Action_Attack, "AI.Action.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Action_Attack_Melee, "AI.Action.Attack.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Action_Attack_Ranged, "AI.Action.Attack.Ranged");
}
