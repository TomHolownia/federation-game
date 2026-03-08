// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Navigation/WaypointComponent.h"
#include "Navigation/WaypointTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaypointComponentDefaults,
	"FederationGame.Navigation.WaypointComponent.HasSensibleDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWaypointComponentDefaults::RunTest(const FString& Parameters)
{
	UWaypointComponent* Comp = NewObject<UWaypointComponent>();
	TestTrue(TEXT("Default bWaypointEnabled should be true"), Comp->bWaypointEnabled);
	TestEqual(TEXT("Default type should be Custom"), Comp->WaypointType, EWaypointType::Custom);
	TestTrue(TEXT("Default MaxVisibleDistance should be 0 (always visible)"), Comp->MaxVisibleDistance == 0.0);
	TestTrue(TEXT("Default DisplayName should be empty"), Comp->DisplayName.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaypointComponentSetActive,
	"FederationGame.Navigation.WaypointComponent.SetActiveTogglesVisibility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWaypointComponentSetActive::RunTest(const FString& Parameters)
{
	UWaypointComponent* Comp = NewObject<UWaypointComponent>();
	TestTrue(TEXT("Should start enabled"), Comp->IsWaypointEnabled());

	Comp->SetWaypointEnabled(false);
	TestFalse(TEXT("Should be disabled after SetWaypointEnabled(false)"), Comp->IsWaypointEnabled());

	Comp->SetWaypointEnabled(true);
	TestTrue(TEXT("Should be enabled again"), Comp->IsWaypointEnabled());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaypointComponentFormatDistanceKm,
	"FederationGame.Navigation.WaypointComponent.FormatDistance_Kilometres",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWaypointComponentFormatDistanceKm::RunTest(const FString& Parameters)
{
	// 5 km = 500,000 UU
	FText Result = UWaypointComponent::FormatDistance(500000.0);
	TestTrue(TEXT("5 km should contain 'km'"), Result.ToString().Contains(TEXT("km")));
	TestTrue(TEXT("5 km should show '5'"), Result.ToString().Contains(TEXT("5")));

	// 150 km = 15,000,000 UU -- should show no decimal places
	FText Result150 = UWaypointComponent::FormatDistance(15000000.0);
	TestTrue(TEXT("150 km should contain 'km'"), Result150.ToString().Contains(TEXT("km")));
	TestTrue(TEXT("150 km should show '150'"), Result150.ToString().Contains(TEXT("150")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaypointComponentFormatDistanceMetres,
	"FederationGame.Navigation.WaypointComponent.FormatDistance_Metres",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWaypointComponentFormatDistanceMetres::RunTest(const FString& Parameters)
{
	// 500 m = 50,000 UU
	FText Result = UWaypointComponent::FormatDistance(50000.0);
	TestTrue(TEXT("500m should contain 'm'"), Result.ToString().Contains(TEXT("m")));
	TestFalse(TEXT("500m should not contain 'km'"), Result.ToString().Contains(TEXT("km")));
	TestTrue(TEXT("500m should show '500'"), Result.ToString().Contains(TEXT("500")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaypointComponentFormatDistanceZero,
	"FederationGame.Navigation.WaypointComponent.FormatDistance_Zero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWaypointComponentFormatDistanceZero::RunTest(const FString& Parameters)
{
	FText Result = UWaypointComponent::FormatDistance(0.0);
	TestTrue(TEXT("0 distance should contain 'm'"), Result.ToString().Contains(TEXT("m")));
	TestTrue(TEXT("0 distance should show '0'"), Result.ToString().Contains(TEXT("0")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaypointComponentGetWaypointLocation,
	"FederationGame.Navigation.WaypointComponent.GetWaypointLocation_NoOwner",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWaypointComponentGetWaypointLocation::RunTest(const FString& Parameters)
{
	UWaypointComponent* Comp = NewObject<UWaypointComponent>();
	TestEqual(TEXT("No owner should return zero vector"), Comp->GetWaypointLocation(), FVector::ZeroVector);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
