// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FederationGameMode.generated.h"

class AFederationGameState;
class AFederationHUD;

/**
 * Default game mode for Federation Game. Uses AFederationCharacter as the default pawn,
 * AFederationGameState for dev diagnostics, and AFederationHUD for on-screen dev info.
 */
UCLASS()
class FEDERATION_API AFederationGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFederationGameMode();
};
