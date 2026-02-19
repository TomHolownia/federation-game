// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Characters/FederationCharacter.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFederationCharacterHasCapsuleAndMovement,
	"FederationGame.Characters.FederationCharacter.HasCapsuleAndMovement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FFederationCharacterHasCapsuleAndMovement::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts().Num() > 0 ? GEngine->GetWorldContexts()[0].World() : nullptr;
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}

	AFederationCharacter* Character = World->SpawnActor<AFederationCharacter>();
	if (!Character)
	{
		AddError(TEXT("Failed to spawn AFederationCharacter"));
		return false;
	}

	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	TestNotNull(TEXT("FederationCharacter must have a capsule component"), Capsule);
	if (Capsule)
	{
		TestTrue(TEXT("Capsule collision should be enabled for walking"), Capsule->GetCollisionEnabled() != ECollisionEnabled::NoCollision);
	}

	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	TestNotNull(TEXT("FederationCharacter must have a character movement component"), Movement);

	Character->Destroy();
	return true;
}

#endif
