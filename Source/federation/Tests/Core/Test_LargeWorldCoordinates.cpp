// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

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

	AActor* Actor = World->SpawnActor<AActor>();
	if (!Actor)
	{
		AddError(TEXT("Failed to spawn actor"));
		return false;
	}

	Actor->SetActorLocation(LargePosition);
	const FVector ReadBack = Actor->GetActorLocation();

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
