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

// ---------------------------------------------------------------------------
// 11. Adaptive streaming radius grows with approach speed
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerAdaptiveStreamingRadiusBySpeed,
	"FederationGame.Planet.PlanetSurfaceStreamer.AdaptiveStreamingRadiusBySpeed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerAdaptiveStreamingRadiusBySpeed::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->TransitionProfile.bUseAdaptiveRadii = true;
	const float PlanetRadius = 100000.f;

	const float RadiusSlow = Comp->ComputeAdaptiveStreamingRadius(PlanetRadius, 500.f);
	const float RadiusFast = Comp->ComputeAdaptiveStreamingRadius(PlanetRadius, 25000.f);

	TestTrue(TEXT("Adaptive streaming radius should grow with approach speed"), RadiusFast > RadiusSlow);

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 12. Adaptive handoff radius grows with approach speed
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerAdaptiveHandoffRadiusBySpeed,
	"FederationGame.Planet.PlanetSurfaceStreamer.AdaptiveHandoffRadiusBySpeed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerAdaptiveHandoffRadiusBySpeed::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Failed to spawn")); return false; }

	Comp->TransitionProfile.bUseAdaptiveRadii = true;
	const float PlanetRadius = 100000.f;

	const float RadiusSlow = Comp->ComputeAdaptiveHandoffRadius(PlanetRadius, 500.f);
	const float RadiusFast = Comp->ComputeAdaptiveHandoffRadius(PlanetRadius, 25000.f);

	TestTrue(TEXT("Adaptive handoff radius should grow with approach speed"), RadiusFast > RadiusSlow);

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 13. Per-planet profiles remain isolated
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerPerPlanetProfileIsolation,
	"FederationGame.Planet.PlanetSurfaceStreamer.PerPlanetProfileIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerPerPlanetProfileIsolation::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* CompA = nullptr;
	UPlanetSurfaceStreamer* CompB = nullptr;
	AActor* ActorA = SpawnActorWithStreamer(World, CompA);
	AActor* ActorB = SpawnActorWithStreamer(World, CompB);
	if (!ActorA || !CompA || !ActorB || !CompB) { AddError(TEXT("Failed to spawn")); return false; }

	CompA->TransitionProfile.BaseStreamingRadiusMultiplier = 1.8f;
	CompB->TransitionProfile.BaseStreamingRadiusMultiplier = 3.6f;

	const float PlanetRadius = 100000.f;
	const float ApproachSpeed = 5000.f;
	const float RadiusA = CompA->ComputeAdaptiveStreamingRadius(PlanetRadius, ApproachSpeed);
	const float RadiusB = CompB->ComputeAdaptiveStreamingRadius(PlanetRadius, ApproachSpeed);

	TestTrue(TEXT("Different per-planet profiles should produce different adaptive radii"), !FMath::IsNearlyEqual(RadiusA, RadiusB, 0.1f));

	ActorA->Destroy();
	ActorB->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 14. Multi-planet state isolation across save/restore
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetSurfaceStreamerMultiPlanetStateIsolation,
	"FederationGame.Planet.PlanetSurfaceStreamer.MultiPlanetStateIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetSurfaceStreamerMultiPlanetStateIsolation::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world context")); return false; }

	UPlanetSurfaceStreamer* CompA = nullptr;
	UPlanetSurfaceStreamer* CompB = nullptr;
	AActor* ActorA = SpawnActorWithStreamer(World, CompA);
	AActor* ActorB = SpawnActorWithStreamer(World, CompB);
	if (!ActorA || !CompA || !ActorB || !CompB) { AddError(TEXT("Failed to spawn")); return false; }

	CompA->SaveSpacePosition(FVector(100.f, 0.f, 0.f), FRotator(0.f, 10.f, 0.f));
	CompB->SaveSpacePosition(FVector(200.f, 0.f, 0.f), FRotator(0.f, 20.f, 0.f));

	FVector OutLocA, OutLocB;
	FRotator OutRotA, OutRotB;
	CompA->GetSavedSpacePosition(OutLocA, OutRotA);
	CompB->GetSavedSpacePosition(OutLocB, OutRotB);

	TestTrue(TEXT("Planet A state should remain independent"), OutLocA.Equals(FVector(100.f, 0.f, 0.f), 0.1f));
	TestTrue(TEXT("Planet B state should remain independent"), OutLocB.Equals(FVector(200.f, 0.f, 0.f), 0.1f));
	TestTrue(TEXT("Planet A and B rotations should remain independent"), !OutRotA.Equals(OutRotB, 0.1f));

	ActorA->Destroy();
	ActorB->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// Helper: create a planet actor with known center and radius for coordinate
// mapping tests. Sets bounds so GetPlanetRadiusFromOwner() returns PlanetRadius.
// ---------------------------------------------------------------------------
static AActor* SpawnPlanetWithStreamer(
	UWorld* World,
	UPlanetSurfaceStreamer*& OutComp,
	const FVector& PlanetCenter,
	float PlanetRadius)
{
	AActor* Actor = World->SpawnActor<AActor>();
	if (!Actor) return nullptr;

	Actor->SetActorLocation(PlanetCenter);

	USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("Root"));
	Root->RegisterComponent();
	Actor->SetRootComponent(Root);
	Root->Bounds = FBoxSphereBounds(PlanetCenter, FVector(PlanetRadius), PlanetRadius);

	OutComp = NewObject<UPlanetSurfaceStreamer>(Actor, TEXT("TestStreamer"));
	OutComp->RegisterComponent();
	return Actor;
}

// ===========================================================================
// 5a. Missing planet approach tests
// ===========================================================================

// ---------------------------------------------------------------------------
// 15. ShouldTransitionToSurface: inside handoff radius
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerTransitionInsideHandoff,
	"FederationGame.Planet.PlanetSurfaceStreamer.ShouldTransitionInsideHandoffRadius",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerTransitionInsideHandoff::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, FVector::ZeroVector, 100000.f);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->TransitionProfile.bUseExplicitRadiiOverrides = true;
	Comp->HandoffRadius = 120000.f;

	const float InsideSq = 110000.f * 110000.f;
	TestTrue(TEXT("Inside handoff radius should transition"), Comp->ShouldTransitionToSurface(InsideSq));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 16. ShouldTransitionToSurface: outside handoff radius
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerTransitionOutsideHandoff,
	"FederationGame.Planet.PlanetSurfaceStreamer.ShouldTransitionOutsideHandoffRadius",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerTransitionOutsideHandoff::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, FVector::ZeroVector, 100000.f);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->TransitionProfile.bUseExplicitRadiiOverrides = true;
	Comp->HandoffRadius = 120000.f;

	const float OutsideSq = 200000.f * 200000.f;
	TestFalse(TEXT("Outside handoff radius should not transition"), Comp->ShouldTransitionToSurface(OutsideSq));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 17. Explicit streaming radius override
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerExplicitStreamingOverride,
	"FederationGame.Planet.PlanetSurfaceStreamer.ExplicitStreamingRadiusOverride",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerExplicitStreamingOverride::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, FVector::ZeroVector, 100000.f);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->StreamingRadius = 500000.f;
	Comp->TransitionProfile.bUseExplicitRadiiOverrides = true;

	TestTrue(TEXT("Explicit override returns StreamingRadius"),
		FMath::IsNearlyEqual(Comp->GetEffectiveStreamingRadius(), 500000.f, 1.f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 18. Adaptive streaming radius when override disabled
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerAdaptiveStreamingWhenOverrideDisabled,
	"FederationGame.Planet.PlanetSurfaceStreamer.AdaptiveStreamingWhenOverrideDisabled",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerAdaptiveStreamingWhenOverrideDisabled::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, FVector::ZeroVector, 100000.f);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->StreamingRadius = 500000.f;
	Comp->TransitionProfile.bUseExplicitRadiiOverrides = false;
	Comp->TransitionProfile.bUseAdaptiveRadii = true;

	const float Result = Comp->GetEffectiveStreamingRadius();
	TestTrue(TEXT("With override disabled, should NOT return the explicit value"),
		!FMath::IsNearlyEqual(Result, 500000.f, 1.f));
	TestTrue(TEXT("Adaptive result should be positive"), Result > 0.f);

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 19. Explicit handoff radius override
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerExplicitHandoffOverride,
	"FederationGame.Planet.PlanetSurfaceStreamer.ExplicitHandoffRadiusOverride",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerExplicitHandoffOverride::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, FVector::ZeroVector, 100000.f);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->HandoffRadius = 150000.f;
	Comp->TransitionProfile.bUseExplicitRadiiOverrides = true;

	TestTrue(TEXT("Explicit override returns HandoffRadius"),
		FMath::IsNearlyEqual(Comp->GetEffectiveHandoffRadius(), 150000.f, 1.f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 20. Transition lock: first acquire succeeds
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerLockFirstAcquireSucceeds,
	"FederationGame.Planet.PlanetSurfaceStreamer.TransitionLockFirstAcquireSucceeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerLockFirstAcquireSucceeds::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->ReleaseTransitionLock();
	TestTrue(TEXT("First lock acquire should succeed"), Comp->TryAcquireTransitionLock());
	TestTrue(TEXT("bOwnsTransitionLock should be set"), Comp->bOwnsTransitionLock);

	Comp->ReleaseTransitionLock();
	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 21. Transition lock: second streamer blocked
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerLockSecondBlocked,
	"FederationGame.Planet.PlanetSurfaceStreamer.TransitionLockSecondStreamerBlocked",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerLockSecondBlocked::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* CompA = nullptr;
	UPlanetSurfaceStreamer* CompB = nullptr;
	AActor* ActorA = SpawnActorWithStreamer(World, CompA);
	AActor* ActorB = SpawnActorWithStreamer(World, CompB);
	if (!ActorA || !CompA || !ActorB || !CompB) { AddError(TEXT("Spawn failed")); return false; }

	CompA->ReleaseTransitionLock();
	CompB->ReleaseTransitionLock();

	TestTrue(TEXT("A acquires lock"), CompA->TryAcquireTransitionLock());
	TestFalse(TEXT("B cannot acquire while A holds"), CompB->TryAcquireTransitionLock());

	CompA->ReleaseTransitionLock();
	ActorA->Destroy();
	ActorB->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 22. Transition lock: release allows re-acquire
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerLockReleaseAllowsReacquire,
	"FederationGame.Planet.PlanetSurfaceStreamer.TransitionLockReleaseAllowsReacquire",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerLockReleaseAllowsReacquire::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* CompA = nullptr;
	UPlanetSurfaceStreamer* CompB = nullptr;
	AActor* ActorA = SpawnActorWithStreamer(World, CompA);
	AActor* ActorB = SpawnActorWithStreamer(World, CompB);
	if (!ActorA || !CompA || !ActorB || !CompB) { AddError(TEXT("Spawn failed")); return false; }

	CompA->ReleaseTransitionLock();
	CompB->ReleaseTransitionLock();

	CompA->TryAcquireTransitionLock();
	CompA->ReleaseTransitionLock();

	TestTrue(TEXT("B can acquire after A releases"), CompB->TryAcquireTransitionLock());

	CompB->ReleaseTransitionLock();
	ActorA->Destroy();
	ActorB->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 23. ComputeRevealProgress: at fade-start distance → 0
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerRevealProgressAtFadeStart,
	"FederationGame.Planet.PlanetSurfaceStreamer.RevealProgressAtFadeStartIsZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerRevealProgressAtFadeStart::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, FVector::ZeroVector, 100000.f);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->FadeStartMultiplier = 2.0f;
	Comp->TransitionProfile.bUseExplicitRadiiOverrides = true;
	Comp->HandoffRadius = 110000.f;

	const float FadeStart = Comp->GetEffectiveFadeStartRadius();
	const float Progress = Comp->ComputeRevealProgress(FadeStart);
	TestTrue(TEXT("Progress at fade-start distance should be ~0"),
		FMath::IsNearlyEqual(Progress, 0.f, 0.01f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 24. ComputeRevealProgress: at full-fade distance → 1
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerRevealProgressAtFullFade,
	"FederationGame.Planet.PlanetSurfaceStreamer.RevealProgressAtFullFadeIsOne",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerRevealProgressAtFullFade::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, FVector::ZeroVector, 100000.f);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->FadeStartMultiplier = 2.0f;
	Comp->TransitionProfile.bUseExplicitRadiiOverrides = true;
	Comp->HandoffRadius = 110000.f;

	const float FullFade = Comp->GetEffectiveFullFadeRadius();
	const float Progress = Comp->ComputeRevealProgress(FullFade);
	TestTrue(TEXT("Progress at full-fade distance should be ~1"),
		FMath::IsNearlyEqual(Progress, 1.f, 0.01f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 25. ComputeRevealProgress: midway → ~0.5
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerRevealProgressMidway,
	"FederationGame.Planet.PlanetSurfaceStreamer.RevealProgressMidwayIsHalf",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerRevealProgressMidway::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, FVector::ZeroVector, 100000.f);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->FadeStartMultiplier = 2.0f;
	Comp->FadeEaseExponent = 1.f;
	Comp->TransitionProfile.bUseExplicitRadiiOverrides = true;
	Comp->HandoffRadius = 110000.f;

	const float FadeStart = Comp->GetEffectiveFadeStartRadius();
	const float FullFade = Comp->GetEffectiveFullFadeRadius();
	const float Mid = (FadeStart + FullFade) * 0.5f;
	const float Progress = Comp->ComputeRevealProgress(Mid);
	TestTrue(TEXT("Progress midway should be ~0.5"),
		FMath::IsNearlyEqual(Progress, 0.5f, 0.05f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 26. Fade disabled: ComputeRevealProgress always returns 1
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerFadeDisabledRevealAlwaysOne,
	"FederationGame.Planet.PlanetSurfaceStreamer.FadeDisabledRevealAlwaysOne",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerFadeDisabledRevealAlwaysOne::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, FVector::ZeroVector, 100000.f);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->FadeStartMultiplier = 0.f;
	Comp->TransitionProfile.bUseExplicitRadiiOverrides = true;
	Comp->HandoffRadius = 110000.f;

	TestTrue(TEXT("With fade disabled, reveal at far distance is 1"),
		FMath::IsNearlyEqual(Comp->ComputeRevealProgress(500000.f), 1.f, 0.01f));
	TestTrue(TEXT("With fade disabled, reveal at near distance is 1"),
		FMath::IsNearlyEqual(Comp->ComputeRevealProgress(100.f), 1.f, 0.01f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 27. Adaptive radii: zero speed → base radius
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerAdaptiveZeroSpeed,
	"FederationGame.Planet.PlanetSurfaceStreamer.AdaptiveRadiiZeroSpeedGivesBase",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerAdaptiveZeroSpeed::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->TransitionProfile.bUseAdaptiveRadii = true;
	const float R = 100000.f;
	const float Expected = R * Comp->TransitionProfile.BaseStreamingRadiusMultiplier;

	const float Result = Comp->ComputeAdaptiveStreamingRadius(R, 0.f);
	TestTrue(TEXT("Zero speed should give base radius (no speed margin)"),
		FMath::IsNearlyEqual(Result, Expected, 1.f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 28. Adaptive radii: speed beyond max is clamped
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerAdaptiveSpeedClamp,
	"FederationGame.Planet.PlanetSurfaceStreamer.AdaptiveRadiiSpeedClampedToMax",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerAdaptiveSpeedClamp::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->TransitionProfile.bUseAdaptiveRadii = true;
	const float R = 100000.f;

	const float AtMax = Comp->ComputeAdaptiveStreamingRadius(R, Comp->TransitionProfile.MaxAssumedApproachSpeed);
	const float BeyondMax = Comp->ComputeAdaptiveStreamingRadius(R, Comp->TransitionProfile.MaxAssumedApproachSpeed * 10.f);

	TestTrue(TEXT("Speed beyond max should clamp to same result"),
		FMath::IsNearlyEqual(AtMax, BeyondMax, 1.f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 29. Adaptive radii: tiny planet radius → positive, no div-by-zero
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerAdaptiveTinyPlanet,
	"FederationGame.Planet.PlanetSurfaceStreamer.AdaptiveRadiiTinyPlanetPositive",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerAdaptiveTinyPlanet::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnActorWithStreamer(World, Comp);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->TransitionProfile.bUseAdaptiveRadii = true;
	const float TinyR = 0.5f;

	const float SR = Comp->ComputeAdaptiveStreamingRadius(TinyR, 1000.f);
	const float HR = Comp->ComputeAdaptiveHandoffRadius(TinyR, 1000.f);

	TestTrue(TEXT("Streaming radius for tiny planet is positive"), SR > 0.f);
	TestTrue(TEXT("Handoff radius for tiny planet is positive"), HR > 0.f);
	TestTrue(TEXT("No NaN in streaming radius"), !FMath::IsNaN(SR));
	TestTrue(TEXT("No NaN in handoff radius"), !FMath::IsNaN(HR));

	Actor->Destroy();
	return true;
}

// ===========================================================================
// 5b. Coordinate mapping tests (TDD)
// ===========================================================================

// ---------------------------------------------------------------------------
// 30. Tangent frame: orthonormal
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerTangentFrameOrthonormal,
	"FederationGame.Planet.PlanetSurfaceStreamer.TangentFrameOrthonormal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerTangentFrameOrthonormal::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->SurfaceLevelWorldOrigin = PlanetCenter + FVector(0, 0, R);
	Comp->ComputeTangentFrame(PlanetCenter);

	TestTrue(TEXT("TangentNormal is unit length"),
		FMath::IsNearlyEqual(Comp->TangentNormal.Size(), 1.f, 0.001f));
	TestTrue(TEXT("TangentX is unit length"),
		FMath::IsNearlyEqual(Comp->TangentX.Size(), 1.f, 0.001f));
	TestTrue(TEXT("TangentY is unit length"),
		FMath::IsNearlyEqual(Comp->TangentY.Size(), 1.f, 0.001f));

	TestTrue(TEXT("Normal dot X is ~0"),
		FMath::IsNearlyZero(FVector::DotProduct(Comp->TangentNormal, Comp->TangentX), 0.001f));
	TestTrue(TEXT("Normal dot Y is ~0"),
		FMath::IsNearlyZero(FVector::DotProduct(Comp->TangentNormal, Comp->TangentY), 0.001f));
	TestTrue(TEXT("X dot Y is ~0"),
		FMath::IsNearlyZero(FVector::DotProduct(Comp->TangentX, Comp->TangentY), 0.001f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 31. Tangent frame: normal matches radial direction
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerTangentNormalMatchesRadial,
	"FederationGame.Planet.PlanetSurfaceStreamer.TangentNormalMatchesRadial",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerTangentNormalMatchesRadial::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(1000, 2000, -5000);
	const float R = 80000.f;
	const FVector TangentPoint = PlanetCenter + FVector(1, 1, 1).GetSafeNormal() * R;

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	const FVector ExpectedNormal = (TangentPoint - PlanetCenter).GetSafeNormal();
	TestTrue(TEXT("TangentNormal should match radial direction"),
		Comp->TangentNormal.Equals(ExpectedNormal, 0.001f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 32. Tangent frame: stable for side approach
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerTangentFrameSideApproach,
	"FederationGame.Planet.PlanetSurfaceStreamer.TangentFrameStableForSideApproach",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerTangentFrameSideApproach::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	Comp->SurfaceLevelWorldOrigin = PlanetCenter + FVector(R, 0, 0);
	Comp->ComputeTangentFrame(PlanetCenter);

	TestTrue(TEXT("Side approach: Normal is unit"),
		FMath::IsNearlyEqual(Comp->TangentNormal.Size(), 1.f, 0.001f));
	TestTrue(TEXT("Side approach: frame is orthogonal"),
		FMath::IsNearlyZero(FVector::DotProduct(Comp->TangentNormal, Comp->TangentX), 0.001f));
	TestTrue(TEXT("Side approach: X is unit"),
		FMath::IsNearlyEqual(Comp->TangentX.Size(), 1.f, 0.001f));
	TestTrue(TEXT("Side approach: Y is unit"),
		FMath::IsNearlyEqual(Comp->TangentY.Size(), 1.f, 0.001f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 33. Tangent frame: degenerate Z-axis approach uses ForwardVector hint
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerTangentFrameZAxisDegenerate,
	"FederationGame.Planet.PlanetSurfaceStreamer.TangentFrameHandlesZAxisApproach",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerTangentFrameZAxisDegenerate::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;

	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	// Directly above (Z+): the UpVector hint degenerates, should use ForwardVector instead
	Comp->SurfaceLevelWorldOrigin = PlanetCenter + FVector(0, 0, R);
	Comp->ComputeTangentFrame(PlanetCenter);

	TestTrue(TEXT("Z-axis: frame is orthonormal N"),
		FMath::IsNearlyEqual(Comp->TangentNormal.Size(), 1.f, 0.001f));
	TestTrue(TEXT("Z-axis: frame is orthonormal X"),
		FMath::IsNearlyEqual(Comp->TangentX.Size(), 1.f, 0.001f));
	TestTrue(TEXT("Z-axis: N dot X is ~0"),
		FMath::IsNearlyZero(FVector::DotProduct(Comp->TangentNormal, Comp->TangentX), 0.001f));
	TestTrue(TEXT("Z-axis: N dot Y is ~0"),
		FMath::IsNearlyZero(FVector::DotProduct(Comp->TangentNormal, Comp->TangentY), 0.001f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 34. SpaceToSurface: center mapping
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerSpaceToSurfaceCenter,
	"FederationGame.Planet.PlanetSurfaceStreamer.SpaceToSurfaceCenterMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerSpaceToSurfaceCenter::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	// Player on the sphere surface right at the tangent point
	const FVector SpacePos = TangentPoint;
	const FVector SurfacePos = Comp->SpaceToSurfacePosition(SpacePos);

	// XY should be ~0 (center of level), Z clamped to min altitude (300)
	TestTrue(TEXT("Center X is ~0"),
		FMath::IsNearlyZero(SurfacePos.X - TangentPoint.X, 10.f));
	TestTrue(TEXT("Center Y is ~0"),
		FMath::IsNearlyZero(SurfacePos.Y - TangentPoint.Y, 10.f));
	TestTrue(TEXT("Altitude clamped to min 300"),
		SurfacePos.Z >= TangentPoint.Z + 290.f);

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 35. SpaceToSurface: altitude preservation
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerSpaceToSurfaceAltitude,
	"FederationGame.Planet.PlanetSurfaceStreamer.SpaceToSurfaceAltitudePreserved",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerSpaceToSurfaceAltitude::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	// Player 10000 UU above the sphere surface, directly above tangent point
	const float AltitudeAboveSurface = 10000.f;
	const FVector SpacePos = PlanetCenter + FVector(0, 0, R + AltitudeAboveSurface);
	const FVector SurfacePos = Comp->SpaceToSurfacePosition(SpacePos);

	const float ResultAltitude = SurfacePos.Z - TangentPoint.Z;
	TestTrue(TEXT("Altitude on surface level should match altitude above sphere"),
		FMath::IsNearlyEqual(ResultAltitude, AltitudeAboveSurface, 100.f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 36. SpaceToSurface: lateral offset produces non-zero XY
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerSpaceToSurfaceLateral,
	"FederationGame.Planet.PlanetSurfaceStreamer.SpaceToSurfaceLateralOffset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerSpaceToSurfaceLateral::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	// Player 10000 UU east and 10000 above surface on the sphere
	const FVector OffsetDir = FVector(10000, 0, R + 5000).GetSafeNormal();
	const FVector SpacePos = PlanetCenter + OffsetDir * (R + 5000.f);
	const FVector SurfacePos = Comp->SpaceToSurfacePosition(SpacePos);

	const FVector LocalOffset = SurfacePos - TangentPoint;
	const float LateralDistSq = LocalOffset.X * LocalOffset.X + LocalOffset.Y * LocalOffset.Y;
	TestTrue(TEXT("Lateral displacement should produce non-zero XY offset on surface"),
		LateralDistSq > 100.f);

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 37. SpaceToSurface: proportional offset (double angle → double XY)
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerSpaceToSurfaceProportional,
	"FederationGame.Planet.PlanetSurfaceStreamer.SpaceToSurfaceProportionalOffset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerSpaceToSurfaceProportional::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	// Small angular offset
	const float Angle1 = 0.05f;
	const float Angle2 = 0.10f;
	const FVector Dir1 = FVector(FMath::Sin(Angle1), 0, FMath::Cos(Angle1)).GetSafeNormal();
	const FVector Dir2 = FVector(FMath::Sin(Angle2), 0, FMath::Cos(Angle2)).GetSafeNormal();
	const FVector Pos1 = PlanetCenter + Dir1 * (R + 5000.f);
	const FVector Pos2 = PlanetCenter + Dir2 * (R + 5000.f);

	const FVector Surf1 = Comp->SpaceToSurfacePosition(Pos1);
	const FVector Surf2 = Comp->SpaceToSurfacePosition(Pos2);

	const float Lateral1 = FVector2D(Surf1.X - TangentPoint.X, Surf1.Y - TangentPoint.Y).Size();
	const float Lateral2 = FVector2D(Surf2.X - TangentPoint.X, Surf2.Y - TangentPoint.Y).Size();

	// Doubling the angle should roughly double the offset
	const float Ratio = Lateral2 / FMath::Max(1.f, Lateral1);
	TestTrue(TEXT("Double angle should approximately double lateral offset"),
		FMath::IsNearlyEqual(Ratio, 2.f, 0.3f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 38. SpaceToSurface: minimum altitude clamp
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerSpaceToSurfaceMinAltitude,
	"FederationGame.Planet.PlanetSurfaceStreamer.SpaceToSurfaceMinAltitudeClamp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerSpaceToSurfaceMinAltitude::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	// Player right at the sphere surface (altitude 0)
	const FVector SpacePos = TangentPoint;
	const FVector SurfacePos = Comp->SpaceToSurfacePosition(SpacePos);

	const float Altitude = SurfacePos.Z - TangentPoint.Z;
	TestTrue(TEXT("Min altitude should be >= 300"),
		Altitude >= 290.f);

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 39. SurfaceToSpace: center mapping
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerSurfaceToSpaceCenter,
	"FederationGame.Planet.PlanetSurfaceStreamer.SurfaceToSpaceCenterMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerSurfaceToSpaceCenter::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	// Level origin → should map back to the tangent point on the sphere
	const FVector SurfacePos = TangentPoint;
	const FVector SpacePos = Comp->SurfaceToSpacePosition(SurfacePos);

	TestTrue(TEXT("Center of level maps to tangent point on sphere"),
		SpacePos.Equals(TangentPoint, 100.f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 40. SurfaceToSpace: altitude preservation
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerSurfaceToSpaceAltitude,
	"FederationGame.Planet.PlanetSurfaceStreamer.SurfaceToSpaceAltitudePreserved",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerSurfaceToSpaceAltitude::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	const float TestAltitude = 5000.f;
	const FVector SurfacePos = TangentPoint + FVector(0, 0, TestAltitude);
	const FVector SpacePos = Comp->SurfaceToSpacePosition(SurfacePos);

	const float SpaceAltitude = FVector::Dist(SpacePos, PlanetCenter) - R;
	TestTrue(TEXT("Space altitude should match surface Z offset"),
		FMath::IsNearlyEqual(SpaceAltitude, TestAltitude, 100.f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 41. SurfaceToSpace: lateral offset maps to rotated direction
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerSurfaceToSpaceLateral,
	"FederationGame.Planet.PlanetSurfaceStreamer.SurfaceToSpaceLateralOffset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerSurfaceToSpaceLateral::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	// Move 5000 UU in localX on the surface, at altitude 1000
	const FVector SurfacePos = TangentPoint + FVector(5000, 0, 1000);
	const FVector SpacePos = Comp->SurfaceToSpacePosition(SurfacePos);

	// The space position should not be directly above the tangent point
	const FVector DirFromCenter = (SpacePos - PlanetCenter).GetSafeNormal();
	const FVector TangentDir = (TangentPoint - PlanetCenter).GetSafeNormal();
	const float DotWithTangent = FVector::DotProduct(DirFromCenter, TangentDir);
	TestTrue(TEXT("Lateral offset should rotate direction away from tangent"),
		DotWithTangent < 0.999f);
	TestTrue(TEXT("Direction should still be valid (not NaN)"),
		!DirFromCenter.ContainsNaN());

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 42. SurfaceToSpace: large offset safety (no NaN)
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerSurfaceToSpaceLargeOffset,
	"FederationGame.Planet.PlanetSurfaceStreamer.SurfaceToSpaceLargeOffsetNoNaN",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerSurfaceToSpaceLargeOffset::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	const FVector SurfacePos = TangentPoint + FVector(200000, 200000, 5000);
	const FVector SpacePos = Comp->SurfaceToSpacePosition(SurfacePos);

	TestTrue(TEXT("Large offset should not produce NaN"),
		!SpacePos.ContainsNaN());
	TestTrue(TEXT("Large offset result should have finite magnitude"),
		SpacePos.Size() < 1.0e10f);

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 43. Round-trip: Space → Surface → Space
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerRoundTripSpaceSurfaceSpace,
	"FederationGame.Planet.PlanetSurfaceStreamer.RoundTripSpaceToSurfaceToSpace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerRoundTripSpaceSurfaceSpace::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	// Player above the sphere at a small angular offset, well above min altitude
	const FVector OriginalDir = FVector(5000, 3000, R).GetSafeNormal();
	const FVector OriginalPos = PlanetCenter + OriginalDir * (R + 8000.f);

	const FVector SurfPos = Comp->SpaceToSurfacePosition(OriginalPos);
	const FVector BackToSpace = Comp->SurfaceToSpacePosition(SurfPos);

	TestTrue(TEXT("Round-trip Space→Surface→Space should recover original position"),
		OriginalPos.Equals(BackToSpace, 200.f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 44. Round-trip: Surface → Space → Surface
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerRoundTripSurfaceSpaceSurface,
	"FederationGame.Planet.PlanetSurfaceStreamer.RoundTripSurfaceToSpaceToSurface",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerRoundTripSurfaceSpaceSurface::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	// Player at a surface position with lateral offset
	const FVector OriginalSurf = TangentPoint + FVector(3000, 4000, 6000);
	const FVector SpacePos = Comp->SurfaceToSpacePosition(OriginalSurf);
	const FVector BackToSurf = Comp->SpaceToSurfacePosition(SpacePos);

	TestTrue(TEXT("Round-trip Surface→Space→Surface should recover original position"),
		OriginalSurf.Equals(BackToSurf, 200.f));

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 45. Round-trip at different altitudes
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerRoundTripDifferentAltitudes,
	"FederationGame.Planet.PlanetSurfaceStreamer.RoundTripAtDifferentAltitudes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerRoundTripDifferentAltitudes::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	const float Altitudes[] = { 500.f, 5000.f, 50000.f };
	for (float Alt : Altitudes)
	{
		const FVector SurfPos = TangentPoint + FVector(2000, 1000, Alt);
		const FVector SpacePos = Comp->SurfaceToSpacePosition(SurfPos);
		const FVector BackToSurf = Comp->SpaceToSurfacePosition(SpacePos);

		TestTrue(FString::Printf(TEXT("Round-trip at altitude %.0f should recover"), Alt),
			SurfPos.Equals(BackToSurf, 300.f));
	}

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 46. Round-trip at different lateral offsets
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerRoundTripDifferentLateral,
	"FederationGame.Planet.PlanetSurfaceStreamer.RoundTripAtDifferentLateralOffsets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerRoundTripDifferentLateral::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	const float Offsets[] = { 0.f, 1000.f, 10000.f, 50000.f };
	for (float Off : Offsets)
	{
		const FVector SurfPos = TangentPoint + FVector(Off, Off * 0.5f, 5000);
		const FVector SpacePos = Comp->SurfaceToSpacePosition(SurfPos);
		const FVector BackToSurf = Comp->SpaceToSurfacePosition(SpacePos);

		// Tolerance grows with offset due to tangent-plane projection distortion
		const float Tolerance = FMath::Max(300.f, Off * 0.05f);
		TestTrue(FString::Printf(TEXT("Round-trip at lateral offset %.0f should recover"), Off),
			SurfPos.Equals(BackToSurf, Tolerance));
	}

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 47. Round-trip with different planet sizes
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerRoundTripDifferentPlanetSizes,
	"FederationGame.Planet.PlanetSurfaceStreamer.RoundTripWithDifferentPlanetSizes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerRoundTripDifferentPlanetSizes::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const float Radii[] = { 50000.f, 250000.f, 500000.f };
	for (float R : Radii)
	{
		const FVector PlanetCenter(0, 0, 0);
		UPlanetSurfaceStreamer* Comp = nullptr;
		AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
		if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); continue; }

		const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
		Comp->SurfaceLevelWorldOrigin = TangentPoint;
		Comp->ComputeTangentFrame(PlanetCenter);

		const FVector SurfPos = TangentPoint + FVector(5000, 3000, 8000);
		const FVector SpacePos = Comp->SurfaceToSpacePosition(SurfPos);
		const FVector BackToSurf = Comp->SpaceToSurfacePosition(SpacePos);

		TestTrue(FString::Printf(TEXT("Round-trip with R=%.0f should recover"), R),
			SurfPos.Equals(BackToSurf, 300.f));

		Actor->Destroy();
	}

	return true;
}

// ---------------------------------------------------------------------------
// 48. Integration: different approach angles yield different surface positions
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerDifferentApproachAngles,
	"FederationGame.Planet.PlanetSurfaceStreamer.DifferentApproachAnglesGiveDifferentPositions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerDifferentApproachAngles::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	// Directly above vs 45-degree approach
	const FVector DirectlyAbove = PlanetCenter + FVector(0, 0, R + 10000);
	const FVector FromAngle = PlanetCenter + FVector(10000, 0, R + 5000).GetSafeNormal() * (R + 10000);

	const FVector SurfA = Comp->SpaceToSurfacePosition(DirectlyAbove);
	const FVector SurfB = Comp->SpaceToSurfacePosition(FromAngle);

	const float LateralA = FVector2D(SurfA.X - TangentPoint.X, SurfA.Y - TangentPoint.Y).Size();
	const float LateralB = FVector2D(SurfB.X - TangentPoint.X, SurfB.Y - TangentPoint.Y).Size();

	TestTrue(TEXT("Angled approach should produce larger lateral offset than direct"),
		LateralB > LateralA + 100.f);

	Actor->Destroy();
	return true;
}

// ---------------------------------------------------------------------------
// 49. Integration: move on surface, exit, re-enter → same surface position
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlanetStreamerMoveExitReenter,
	"FederationGame.Planet.PlanetSurfaceStreamer.MoveOnSurfaceExitReenterSamePosition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlanetStreamerMoveExitReenter::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World) { AddError(TEXT("No world")); return false; }

	const FVector PlanetCenter(0, 0, 0);
	const float R = 100000.f;
	UPlanetSurfaceStreamer* Comp = nullptr;
	AActor* Actor = SpawnPlanetWithStreamer(World, Comp, PlanetCenter, R);
	if (!Actor || !Comp) { AddError(TEXT("Spawn failed")); return false; }

	const FVector TangentPoint = PlanetCenter + FVector(0, 0, R);
	Comp->SurfaceLevelWorldOrigin = TangentPoint;
	Comp->ComputeTangentFrame(PlanetCenter);

	// Player walks 10000 UU on surface, then exits
	const FVector SurfaceAfterWalk = TangentPoint + FVector(10000, 0, 500);
	const FVector ExitSpacePos = Comp->SurfaceToSpacePosition(SurfaceAfterWalk);

	// Re-enter from the same space position
	const FVector ReenterSurface = Comp->SpaceToSurfacePosition(ExitSpacePos);

	TestTrue(TEXT("Re-entering from exit position should land at same surface spot"),
		SurfaceAfterWalk.Equals(ReenterSurface, 200.f));

	Actor->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
