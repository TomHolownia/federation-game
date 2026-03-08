// Copyright Federation Game. All Rights Reserved.

#include "Core/FederationGameState.h"

AFederationGameState::AFederationGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DebugStreamingState = TEXT("Idle");
	DebugStreamingLevelName = TEXT("");
	DebugJetpackEnabled = false;
	DebugJetpackBoost = false;
}