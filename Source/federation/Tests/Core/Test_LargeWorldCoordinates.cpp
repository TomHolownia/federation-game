// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test that large world coordinates are preserved (LWC: double-precision positions).
 * Spawns an actor far from origin and checks that GetActorLocation() matches.
 * If LWC is disabled, 32-bit float would lose precision at this scale.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLargeWorldCoordinatesPreservePosition,
	"FederationGame.Core.LargeWorldCoordinates.PreservePositionAtLargeScale",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FLargeWorldCoordinatesPreservePosition::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine ? GEngine->GetWorldContexts()[0].World() : nullptr;
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}

	// 100 million units - with 32-bit float, precision here is ~1 unit; with LWC (double) we get sub-millimetre
	const FVector LargePosition(1e8, 2e8, -5e7);
	const FTransform SpawnTransform(LargePosition);

	// Spawn actor at the large position directly using transform
	AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnTransform);
	if (!Actor)
	{
		AddError(TEXT("Failed to spawn actor"));
		return false;
	}

	// AActor needs a root component for GetActorLocation() to work correctly
	// Create a simple scene component and set it as root BEFORE setting location
	USceneComponent* RootComp = NewObject<USceneComponent>(Actor, USceneComponent::StaticClass(), TEXT("RootComponent"));
	Actor->SetRootComponent(RootComp);
	RootComp->RegisterComponent();
	
	// Set the location on the root component directly, then verify via actor
	RootComp->SetWorldLocation(LargePosition, false, nullptr, ETeleportType::TeleportPhysics);
	
	// Also set via actor API to ensure consistency
	Actor->SetActorLocation(LargePosition, false, nullptr, ETeleportType::TeleportPhysics);

	// Verify the position was set correctly - check root component location directly
	const FVector ReadBack = RootComp->GetComponentLocation();

	Actor->Destroy();

	// With LWC we expect position to be preserved within a small tolerance (e.g. 0.01 units)
	const double Tolerance = 0.01;
	const double Delta = FVector::Dist(LargePosition, ReadBack);
	if (Delta > Tolerance)
	{
		AddError(FString::Printf(TEXT("Large position not preserved: set %s, got %s (delta %f)"),
			*LargePosition.ToString(), *ReadBack.ToString(), Delta));
		return false;
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
