// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FederationGameState.generated.h"

/**
 * Game state for Federation Game. Holds developer diagnostics and other
 * shared state that can be read by HUD, subsystems, etc.
 */
UCLASS()
class FEDERATION_API AFederationGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AFederationGameState(const FObjectInitializer& ObjectInitializer);

	/** Current planet streaming state for dev HUD (Idle, Loading, OnSurface, Unloading). */
	UPROPERTY(BlueprintReadOnly, Category = "Dev")
	FString DebugStreamingState;

	/** Name of the streamed level when relevant (e.g. "PlanetSurface_Test"). */
	UPROPERTY(BlueprintReadOnly, Category = "Dev")
	FString DebugStreamingLevelName;

	/** Whether the player's jetpack is currently enabled. */
	UPROPERTY(BlueprintReadOnly, Category = "Dev")
	bool DebugJetpackEnabled = false;

	/** Whether the player's jetpack boost (C) is currently on. */
	UPROPERTY(BlueprintReadOnly, Category = "Dev")
	bool DebugJetpackBoost = false;

	/** Optional: add more dev diagnostics here (e.g. current planet name, FPS). */
};