// Copyright Federation Game. All Rights Reserved.

#include "Core/FederationGameMode.h"
#include "Character/FederationCharacter.h"

AFederationGameMode::AFederationGameMode()
{
	DefaultPawnClass = AFederationCharacter::StaticClass();
}
