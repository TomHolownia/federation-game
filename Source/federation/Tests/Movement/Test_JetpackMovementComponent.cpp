// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Character/FederationCharacter.h"
#include "Movement/JetpackMovementComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJetpackComponentAttachedToFederationCharacter,
	"FederationGame.Movement.JetpackMovementComponent.AttachedToFederationCharacter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FJetpackComponentAttachedToFederationCharacter::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	AFederationCharacter* Character = World->SpawnActor<AFederationCharacter>();
	if (!Character) { AddError(TEXT("Failed to spawn character")); return false; }

	TestNotNull(TEXT("Character should own a jetpack component"), Character->JetpackComponent.Get());
	TestFalse(TEXT("Jetpack starts disabled"), Character->JetpackComponent->IsJetpackEnabled());

	Character->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJetpackComponentActivationSetsFlyingMode,
	"FederationGame.Movement.JetpackMovementComponent.ActivationSetsFlyingMode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FJetpackComponentActivationSetsFlyingMode::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	AFederationCharacter* Character = World->SpawnActor<AFederationCharacter>();
	if (!Character || !Character->JetpackComponent) { AddError(TEXT("Missing character or jetpack component")); return false; }

	Character->JetpackComponent->MaxJetpackSpeed = 7777.f;
	Character->JetpackComponent->ActivateJetpack();

	TestTrue(TEXT("Jetpack should be enabled"), Character->JetpackComponent->IsJetpackEnabled());
	TestTrue(TEXT("Movement mode should be flying"), Character->GetCharacterMovement()->MovementMode == MOVE_Flying);
	TestEqual(TEXT("Max fly speed should come from component tuning"), Character->GetCharacterMovement()->MaxFlySpeed, 7777.f);

	Character->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJetpackComponentDeactivationReturnsToFalling,
	"FederationGame.Movement.JetpackMovementComponent.DeactivationReturnsToFalling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FJetpackComponentDeactivationReturnsToFalling::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	AFederationCharacter* Character = World->SpawnActor<AFederationCharacter>();
	if (!Character || !Character->JetpackComponent) { AddError(TEXT("Missing character or jetpack component")); return false; }

	Character->JetpackComponent->ActivateJetpack();
	Character->JetpackComponent->DeactivateJetpack();

	TestFalse(TEXT("Jetpack should be disabled"), Character->JetpackComponent->IsJetpackEnabled());
	TestTrue(TEXT("Movement mode should return to falling"), Character->GetCharacterMovement()->MovementMode == MOVE_Falling);

	Character->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
