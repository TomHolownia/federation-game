// Copyright Federation Game. All Rights Reserved.

#include "FederationGameModeBase.h"
#include "Characters/FederationCharacter.h"

AFederationGameModeBase::AFederationGameModeBase()
{
	DefaultPawnClass = AFederationCharacter::StaticClass();
}
