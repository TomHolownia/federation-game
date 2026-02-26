// Copyright Federation Game. All Rights Reserved.

#include "Core/FederationGameMode.h"
#include "Core/FederationGameState.h"
#include "Core/FederationHUD.h"
#include "Character/FederationCharacter.h"

AFederationGameMode::AFederationGameMode()
{
	DefaultPawnClass = AFederationCharacter::StaticClass();
	HUDClass = AFederationHUD::StaticClass();
	GameStateClass = AFederationGameState::StaticClass();
}
