// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FederationGameMode.generated.h"

/**
 * Default game mode for Federation Game. Uses AFederationCharacter as the default pawn.
 */
UCLASS()
class FEDERATION_API AFederationGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFederationGameMode();
};
