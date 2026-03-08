// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Planet/Planet.h"
#include "Planet/PlanetGravitySourceComponent.h"
#include "Planet/PlanetSurfaceStreamer.h"
#include "Navigation/WaypointComponent.h"
#include "Navigation/WaypointTypes.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetActorHasRequiredComponents,
	"FederationGame.Planet.PlanetActor.HasRequiredComponents",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetActorHasRequiredComponents::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	APlanet* Planet = World->SpawnActor<APlanet>();
	if (!Planet) { AddError(TEXT("Failed to spawn APlanet")); return false; }

	TestNotNull(TEXT("Planet should include a surface streamer"), Planet->PlanetSurfaceStreamer.Get());
	TestNotNull(TEXT("Planet should include a gravity source component"), Planet->PlanetGravitySource.Get());
	TestTrue(TEXT("Planet should auto-tag itself as Planet"), Planet->Tags.Contains(FName(TEXT("Planet"))));
	TestNotNull(TEXT("Planet should include a waypoint component"), Planet->WaypointComp.Get());

	if (Planet->WaypointComp)
	{
		TestEqual(TEXT("Planet waypoint type should be Planet"), Planet->WaypointComp->WaypointType, EWaypointType::Planet);
		TestFalse(TEXT("Planet waypoint should have a default display name"), Planet->WaypointComp->DisplayName.IsEmpty());
	}

	Planet->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
