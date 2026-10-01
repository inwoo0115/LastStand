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
}
