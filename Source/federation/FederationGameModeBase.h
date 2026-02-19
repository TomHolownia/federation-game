// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FederationGameModeBase.generated.h"

/**
 * Default Game Mode (FED-030). Spawns AFederationCharacter as the default pawn.
 */
UCLASS(Blueprintable, Category = "Federation")
class FEDERATION_API AFederationGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFederationGameModeBase();
};
