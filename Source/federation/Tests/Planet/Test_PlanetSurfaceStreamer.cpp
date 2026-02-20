// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Planet/PlanetSurfaceStreamer.h"
#include "Planet/PlanetGravityComponent.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMeshActor.h"

#if WITH_DEV_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Helper: create a PlanetSurfaceStreamer on a plain actor.
// ---------------------------------------------------------------------------
static AActor* SpawnActorWithStreamer(UWorld* World, UPlanetSurfaceStreamer*& OutComp)
{
	AActor* Actor = World->SpawnActor<AActor>();
	if (!Actor) return nullptr;

	USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("Root"));
	Root->RegisterComponent();
	Actor->SetRootComponent(Root);

	OutComp = NewObject<UPlanetSurfaceStreamer>(Actor, TEXT("TestStreamer"));
	OutComp->RegisterComponent();
	return Actor;
}

// ---------------------------------------------------------------------------
// 1. ShouldStreamIn: inside radius returns true
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerStreamInInsideRadius,
	"FederationGame.Planet.PlanetSurfaceStreamer.ShouldStreamInInsideRadius",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerStreamInInsideRadius::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->StreamingRadius = 1000.f;

	// Distance squared = 500^2 = 250000, radius squared = 1000^2 = 1000000
	TestTrue(TEXT("Should stream in when inside radius"), Comp->ShouldStreamIn(250000.f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 2. ShouldStreamIn: outside radius returns false
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerStreamInOutsideRadius,
	"FederationGame.Planet.PlanetSurfaceStreamer.ShouldStreamInOutsideRadius",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerStreamInOutsideRadius::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->StreamingRadius = 1000.f;

	// Distance squared = 2000^2 = 4000000, radius squared = 1000^2 = 1000000
	TestFalse(TEXT("Should not stream in when outside radius"), Comp->ShouldStreamIn(4000000.f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 3. ShouldStreamIn: exactly on boundary returns true
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerStreamInOnBoundary,
	"FederationGame.Planet.PlanetSurfaceStreamer.ShouldStreamInOnBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerStreamInOnBoundary::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->StreamingRadius = 1000.f;
	TestTrue(TEXT("Should stream in when exactly on boundary"), Comp->ShouldStreamIn(1000000.f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 4. ShouldStreamOut: above exit altitude
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerStreamOutAboveAltitude,
	"FederationGame.Planet.PlanetSurfaceStreamer.ShouldStreamOutAboveExitAltitude",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerStreamOutAboveAltitude::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->ExitAltitude = 50000.f;
	const FVector SurfaceOrigin(0.f, 0.f, 0.f);
	const FVector PlayerHigh(0.f, 0.f, 60000.f);
	const FVector PlayerLow(0.f, 0.f, 10000.f);

	TestTrue(TEXT("Should stream out when above exit altitude"),
		Comp->ShouldStreamOut(PlayerHigh, SurfaceOrigin));
	TestFalse(TEXT("Should not stream out when below exit altitude"),
		Comp->ShouldStreamOut(PlayerLow, SurfaceOrigin));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 5. State machine: initial state is Idle
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerInitialStateIdle,
	"FederationGame.Planet.PlanetSurfaceStreamer.InitialStateIsIdle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerInitialStateIdle::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	TestTrue(TEXT("Initial state should be Idle"),
		Comp->GetStreamingState() == EPlanetStreamingState::Idle);
	TestFalse(TEXT("IsPlayerOnSurface should be false initially"),
		Comp->IsPlayerOnSurface());

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 6. State transitions: set and query
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerStateTransitions,
	"FederationGame.Planet.PlanetSurfaceStreamer.StateTransitionsWork",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerStateTransitions::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->SetStreamingState(EPlanetStreamingState::Loading);
	TestTrue(TEXT("State should be Loading"),
		Comp->GetStreamingState() == EPlanetStreamingState::Loading);

	Comp->SetStreamingState(EPlanetStreamingState::OnSurface);
	TestTrue(TEXT("State should be OnSurface"),
		Comp->GetStreamingState() == EPlanetStreamingState::OnSurface);
	TestTrue(TEXT("IsPlayerOnSurface should be true when OnSurface"),
		Comp->IsPlayerOnSurface());

	Comp->SetStreamingState(EPlanetStreamingState::Unloading);
	TestTrue(TEXT("State should be Unloading"),
		Comp->GetStreamingState() == EPlanetStreamingState::Unloading);
	TestFalse(TEXT("IsPlayerOnSurface should be false when Unloading"),
		Comp->IsPlayerOnSurface());

	Comp->SetStreamingState(EPlanetStreamingState::Idle);
	TestTrue(TEXT("State should return to Idle"),
		Comp->GetStreamingState() == EPlanetStreamingState::Idle);

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 7. Space position save/restore round-trip
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerPositionSaveRestore,
	"FederationGame.Planet.PlanetSurfaceStreamer.PositionSaveRestoreRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerPositionSaveRestore::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	const FVector OriginalLocation(12345.f, 67890.f, 11111.f);
	const FRotator OriginalRotation(10.f, 20.f, 30.f);

	Comp->SaveSpacePosition(OriginalLocation, OriginalRotation);

	FVector RestoredLocation;
	FRotator RestoredRotation;
	Comp->GetSavedSpacePosition(RestoredLocation, RestoredRotation);

	TestTrue(TEXT("Location should be preserved"),
		RestoredLocation.Equals(OriginalLocation, 0.1f));
	TestTrue(TEXT("Rotation should be preserved"),
		RestoredRotation.Equals(OriginalRotation, 0.1f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 8. GetDistanceToPlayerSquared returns FLT_MAX when no player
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerDistanceNoPlayer,
	"FederationGame.Planet.PlanetSurfaceStreamer.DistanceReturnsMaxWhenNoPlayer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerDistanceNoPlayer::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	const float Dist = Comp->GetDistanceToPlayerSquared();
	TestTrue(TEXT("Distance should be FLT_MAX when no player pawn exists"),
		Dist >= FLT_MAX - 1.f);

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 9. Idle state does not change when no player is nearby
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerIdleNoTransitionWithoutPlayer,
	"FederationGame.Planet.PlanetSurfaceStreamer.IdleNoTransitionWithoutPlayer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerIdleNoTransitionWithoutPlayer::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->StreamingRadius = 1000.f;
	Comp->SetStreamingState(EPlanetStreamingState::Idle);

	Comp->TickComponent(0.016f, LEVELTICK_All, nullptr);

	TestTrue(TEXT("Should remain Idle when no player pawn exists"),
		Comp->GetStreamingState() == EPlanetStreamingState::Idle);

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 10. ShouldStreamOut: lateral distance also counts
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerStreamOutLateralDistance,
	"FederationGame.Planet.PlanetSurfaceStreamer.StreamOutLateralDistanceCounts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerStreamOutLateralDistance::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->ExitAltitude = 50000.f;
	const FVector SurfaceOrigin(0.f, 0.f, 0.f);
	const FVector PlayerFarLateral(60000.f, 0.f, 0.f);

	TestTrue(TEXT("Lateral distance beyond exit altitude should trigger stream out"),
		Comp->ShouldStreamOut(PlayerFarLateral, SurfaceOrigin));

	Actor->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
