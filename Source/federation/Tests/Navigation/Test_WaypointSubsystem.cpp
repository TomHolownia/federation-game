// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Navigation/WaypointSubsystem.h"
#include "Navigation/WaypointComponent.h"
#include "Navigation/WaypointTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaypointSubsystemExists,
	"FederationGame.Navigation.WaypointSubsystem.ExistsInWorld",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWaypointSubsystemExists::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UWaypointSubsystem* Sub = World->GetSubsystem<UWaypointSubsystem>();
	TestNotNull(TEXT("WaypointSubsystem should auto-exist in the world"), Sub);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaypointSubsystemRegisterUnregister,
	"FederationGame.Navigation.WaypointSubsystem.RegisterAndUnregister",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWaypointSubsystemRegisterUnregister::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UWaypointSubsystem* Sub = World->GetSubsystem<UWaypointSubsystem>();
	if (!Sub) { AddError(TEXT("No subsystem")); return false; }

	UWaypointComponent* Comp = NewObject<UWaypointComponent>();
	Comp->bWaypointEnabled = true;

	Sub->RegisterWaypoint(Comp);
	TArray<UWaypointComponent*> Active = Sub->GetAllActiveWaypoints();
	TestEqual(TEXT("Should have 1 active waypoint after register"), Active.Num(), 1);

	Sub->UnregisterWaypoint(Comp);
	Active = Sub->GetAllActiveWaypoints();
	TestEqual(TEXT("Should have 0 active waypoints after unregister"), Active.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaypointSubsystemDuplicateRegister,
	"FederationGame.Navigation.WaypointSubsystem.IgnoresDuplicateRegistration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWaypointSubsystemDuplicateRegister::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UWaypointSubsystem* Sub = World->GetSubsystem<UWaypointSubsystem>();
	if (!Sub) { AddError(TEXT("No subsystem")); return false; }

	UWaypointComponent* Comp = NewObject<UWaypointComponent>();
	Comp->bWaypointEnabled = true;

	Sub->RegisterWaypoint(Comp);
	Sub->RegisterWaypoint(Comp);
	TArray<UWaypointComponent*> Active = Sub->GetAllActiveWaypoints();
	TestEqual(TEXT("Duplicate register should still yield 1 waypoint"), Active.Num(), 1);

	Sub->UnregisterWaypoint(Comp);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaypointSubsystemFilterByType,
	"FederationGame.Navigation.WaypointSubsystem.FiltersByType",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWaypointSubsystemFilterByType::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UWaypointSubsystem* Sub = World->GetSubsystem<UWaypointSubsystem>();
	if (!Sub) { AddError(TEXT("No subsystem")); return false; }

	UWaypointComponent* Planet = NewObject<UWaypointComponent>();
	Planet->bWaypointEnabled = true;
	Planet->WaypointType = EWaypointType::Planet;

	UWaypointComponent* Ship = NewObject<UWaypointComponent>();
	Ship->bWaypointEnabled = true;
	Ship->WaypointType = EWaypointType::Ship;

	Sub->RegisterWaypoint(Planet);
	Sub->RegisterWaypoint(Ship);

	TArray<UWaypointComponent*> Planets = Sub->GetWaypointsByType(EWaypointType::Planet);
	TestEqual(TEXT("Should find 1 planet waypoint"), Planets.Num(), 1);
	TestEqual(TEXT("Planet waypoint should match"), Planets[0], Planet);

	TArray<UWaypointComponent*> Ships = Sub->GetWaypointsByType(EWaypointType::Ship);
	TestEqual(TEXT("Should find 1 ship waypoint"), Ships.Num(), 1);

	TArray<UWaypointComponent*> Stations = Sub->GetWaypointsByType(EWaypointType::Station);
	TestEqual(TEXT("Should find 0 station waypoints"), Stations.Num(), 0);

	Sub->UnregisterWaypoint(Planet);
	Sub->UnregisterWaypoint(Ship);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaypointSubsystemInactiveFiltered,
	"FederationGame.Navigation.WaypointSubsystem.InactiveWaypointsFiltered",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWaypointSubsystemInactiveFiltered::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UWaypointSubsystem* Sub = World->GetSubsystem<UWaypointSubsystem>();
	if (!Sub) { AddError(TEXT("No subsystem")); return false; }

	UWaypointComponent* Comp = NewObject<UWaypointComponent>();
	Comp->bWaypointEnabled = false;

	Sub->RegisterWaypoint(Comp);
	TArray<UWaypointComponent*> Active = Sub->GetAllActiveWaypoints();
	TestEqual(TEXT("Disabled waypoint should not appear in active list"), Active.Num(), 0);

	Sub->UnregisterWaypoint(Comp);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaypointSubsystemNullSafety,
	"FederationGame.Navigation.WaypointSubsystem.HandleNullGracefully",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWaypointSubsystemNullSafety::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UWaypointSubsystem* Sub = World->GetSubsystem<UWaypointSubsystem>();
	if (!Sub) { AddError(TEXT("No subsystem")); return false; }

	Sub->RegisterWaypoint(nullptr);
	Sub->UnregisterWaypoint(nullptr);

	UWaypointComponent* Nearest = Sub->GetNearestWaypoint(FVector::ZeroVector);
	TestNull(TEXT("GetNearestWaypoint with no registrations should return null"), Nearest);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
