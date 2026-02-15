// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Skybox/SkySphere.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"
#include "Components/StaticMeshComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test that ASkySphere spawns with a valid sphere mesh component.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkySphereHasSphereMesh,
	"FederationGame.Skybox.SkySphere.HasSphereMesh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FSkySphereHasSphereMesh::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}

	ASkySphere* SkySphere = World->SpawnActor<ASkySphere>();
	if (!SkySphere)
	{
		AddError(TEXT("Failed to spawn ASkySphere actor"));
		return false;
	}

	TestNotNull(TEXT("SphereMesh component should exist"), SkySphere->SphereMesh);
	if (SkySphere->SphereMesh)
	{
		TestNotNull(TEXT("Sphere mesh should have static mesh set"), SkySphere->SphereMesh->GetStaticMesh());
	}

	SkySphere->Destroy();
	return true;
}

/**
 * Test that ASkySphere::UpdateSkyMaterial does not crash when SkyMaterial is null.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkySphereUpdateSkyMaterialWithNull,
	"FederationGame.Skybox.SkySphere.UpdateSkyMaterialWithNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FSkySphereUpdateSkyMaterialWithNull::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}

	ASkySphere* SkySphere = World->SpawnActor<ASkySphere>();
	if (!SkySphere)
	{
		AddError(TEXT("Failed to spawn ASkySphere actor"));
		return false;
	}

	SkySphere->SkyMaterial = nullptr;
	SkySphere->UpdateSkyMaterial();

	SkySphere->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
