// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Character/FederationCharacter.h"
#include "Planet/PlanetGravityComponent.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFederationCharacterHasFirstPersonCamera,
	"FederationGame.Character.FederationCharacter.HasFirstPersonCamera",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FFederationCharacterHasFirstPersonCamera::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
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

	TestNotNull(TEXT("FirstPersonCameraComponent should exist"), Character->FirstPersonCameraComponent.Get());

	Character->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFederationCharacterHasThirdPersonComponents,
	"FederationGame.Character.FederationCharacter.HasThirdPersonComponents",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FFederationCharacterHasThirdPersonComponents::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
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

	TestNotNull(TEXT("ThirdPersonSpringArm should exist"), Character->ThirdPersonSpringArm.Get());
	TestNotNull(TEXT("ThirdPersonCameraComponent should exist"), Character->ThirdPersonCameraComponent.Get());

	Character->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFederationCharacterMeshVisibleToOwner,
	"FederationGame.Character.FederationCharacter.MeshVisibleToOwner",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FFederationCharacterMeshVisibleToOwner::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
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

	TestNotNull(TEXT("Mesh component should exist"), Character->GetMesh());
	// Character is set up so owner can see their own body (SetOwnerNoSee(false))
	TestFalse(TEXT("Mesh should be visible to owner (first-person body)"),
		Character->GetMesh()->bOwnerNoSee);

	Character->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFederationCharacterToggleViewMode,
	"FederationGame.Character.FederationCharacter.ToggleViewMode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FFederationCharacterToggleViewMode::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
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

	Character->SetThirdPersonView(false);
	TestFalse(TEXT("Should start in first-person"), Character->bUseThirdPersonView);
	TestTrue(TEXT("First-person camera should be active"), Character->FirstPersonCameraComponent->IsActive());
	TestFalse(TEXT("Third-person camera should be inactive"), Character->ThirdPersonCameraComponent->IsActive());

	Character->ToggleViewMode();
	TestTrue(TEXT("After toggle should be third-person"), Character->bUseThirdPersonView);
	TestFalse(TEXT("First-person camera should be inactive"), Character->FirstPersonCameraComponent->IsActive());
	TestTrue(TEXT("Third-person camera should be active"), Character->ThirdPersonCameraComponent->IsActive());

	Character->ToggleViewMode();
	TestFalse(TEXT("After second toggle back to first-person"), Character->bUseThirdPersonView);

	Character->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// Component and character-specific tests
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFederationCharacterHasPlanetGravityComponent,
	"FederationGame.Character.FederationCharacter.HasPlanetGravityComponent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FFederationCharacterHasPlanetGravityComponent::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	AFederationCharacter* Character = World->SpawnActor<AFederationCharacter>();
	if (!Character) { AddError(TEXT("Failed to spawn character")); return false; }

	TestNotNull(TEXT("Character should have a PlanetGravityComponent"),
		Character->GravityComp.Get());

	Character->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFederationCharacterAirControlIsZero,
	"FederationGame.Character.FederationCharacter.AirControlIsZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FFederationCharacterAirControlIsZero::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	AFederationCharacter* Character = World->SpawnActor<AFederationCharacter>();
	if (!Character) { AddError(TEXT("Failed to spawn character")); return false; }

	TestEqual(TEXT("AirControl should be zero for no in-air steering"),
		Character->GetCharacterMovement()->AirControl, 0.f);

	Character->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFederationCharacterFirstPersonCameraRootAbsoluteRotation,
	"FederationGame.Character.FederationCharacter.FirstPersonCameraRootAbsoluteRotation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FFederationCharacterFirstPersonCameraRootAbsoluteRotation::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	AFederationCharacter* Character = World->SpawnActor<AFederationCharacter>();
	if (!Character) { AddError(TEXT("Failed to spawn character")); return false; }

	TestTrue(TEXT("FirstPersonCameraRoot should use absolute rotation"),
		Character->FirstPersonCameraRoot->IsUsingAbsoluteRotation());

	Character->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFederationCharacterSpringArmAbsoluteRotation,
	"FederationGame.Character.FederationCharacter.SpringArmAbsoluteRotation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FFederationCharacterSpringArmAbsoluteRotation::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	AFederationCharacter* Character = World->SpawnActor<AFederationCharacter>();
	if (!Character) { AddError(TEXT("Failed to spawn character")); return false; }

	TestTrue(TEXT("ThirdPersonSpringArm should use absolute rotation"),
		Character->ThirdPersonSpringArm->IsUsingAbsoluteRotation());

	Character->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
