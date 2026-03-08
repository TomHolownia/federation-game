// Copyright Federation Game. All Rights Reserved.

#include "Planet/PlanetSurfaceStreamer.h"
#include "Planet/PlanetGravityComponent.h"
#include "Character/FederationCharacter.h"
#include "Core/FederationGameState.h"
#include "Movement/JetpackMovementComponent.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPath.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/Paths.h"

namespace
{
	TWeakObjectPtr<UPlanetSurfaceStreamer> GPlanetTransitionOwner;
}

UPlanetSurfaceStreamer::UPlanetSurfaceStreamer()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Default path matches docs: create a level at Content/Planets/PlanetSurface_Test. Override in Details if needed.
	SurfaceLevelPath = TEXT("/Game/Planets/PlanetSurface_Test");
}

void UPlanetSurfaceStreamer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (StreamingState)
	{
	case EPlanetStreamingState::Idle:
		UpdateIdleState();
		break;
	case EPlanetStreamingState::Loading:
		UpdateLoadingState();
		break;
	case EPlanetStreamingState::OnSurface:
		UpdateOnSurfaceState();
		break;
	case EPlanetStreamingState::Unloading:
		UpdateUnloadingState();
		break;
	}

	// Reveal/fade starts only after surface loading has begun and level is actually loaded.
	const bool bApproachPhase = StreamingState == EPlanetStreamingState::Idle || StreamingState == EPlanetStreamingState::Loading;
	if (bApproachPhase)
	{
		const bool bCanReveal = FadeStartMultiplier > 0.f
			&& StreamingState == EPlanetStreamingState::Loading
			&& StreamedLevel
			&& StreamedLevel->HasLoadedLevel();

		if (bCanReveal)
		{
			const float DistSq = GetDistanceToPlayerSquared();
			const float Dist = FMath::Sqrt(DistSq);
			UpdatePlanetFade(Dist);
		}
		else
		{
			CurrentRevealProgress = 0.f;
			SetPlanetFadeAlpha(1.f);
			ResetPlanetRevealParams();
		}
	}
}

// ---------------------------------------------------------------------------
// Testable pure logic
// ---------------------------------------------------------------------------

float UPlanetSurfaceStreamer::GetDistanceToPlayerSquared() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return FLT_MAX;

	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn) return FLT_MAX;

	return FVector::DistSquared(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
}

float UPlanetSurfaceStreamer::GetPlanetRadiusFromOwner() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return 0.f;

	const FBox Box = Owner->GetComponentsBoundingBox();
	if (Box.IsValid)
	{
		// GetExtent() returns half-extents (radius per axis), not diameter. We use radius everywhere.
		const FVector Extent = Box.GetExtent();
		return FMath::Max(Extent.X, FMath::Max(Extent.Y, Extent.Z));
	}

	// Fallback: root component bounding sphere (e.g. when box not yet valid)
	if (USceneComponent* Root = Owner->GetRootComponent())
	{
		const float R = Root->Bounds.SphereRadius;
		if (R > 0.f) return R;
	}
	return 0.f;
}

float UPlanetSurfaceStreamer::GetEffectiveHandoffRadius() const
{
	float Result;
	if (TransitionProfile.bUseExplicitRadiiOverrides && HandoffRadius > 0.f)
	{
		Result = HandoffRadius;
	}
	else if (TransitionProfile.bUseAdaptiveRadii)
	{
		const float PlanetRadius = GetPlanetRadiusFromOwner();
		if (PlanetRadius > 0.f)
		{
			Result = ComputeAdaptiveHandoffRadius(PlanetRadius, GetPlayerApproachSpeedTowardPlanet());
		}
		else
		{
			Result = 100000.f;
		}
	}
	else
	{
		const float PlanetRadius = GetPlanetRadiusFromOwner();
		if (PlanetRadius <= 0.f)
		{
			Result = 100000.f;
		}
		else
		{
			const float Margin = FMath::Max(500.f, PlanetRadius * 0.05f);
			Result = PlanetRadius + Margin;
		}
	}

	// Guard: ensure handoff altitude never exceeds 80% of effective exit altitude.
	const float PlanetRadius = GetPlanetRadiusFromOwner();
	if (PlanetRadius > 0.f)
	{
		const float EffectiveExit = GetEffectiveExitAltitude();
		const float MaxHandoffAlt = EffectiveExit * 0.8f;
		const float MaxHandoffRadius = PlanetRadius + MaxHandoffAlt;
		Result = FMath::Min(Result, MaxHandoffRadius);
	}

	return Result;
}

float UPlanetSurfaceStreamer::GetEffectiveExitAltitude() const
{
	const float PlanetRadius = GetPlanetRadiusFromOwner();
	const float AdaptiveAlt = PlanetRadius * FMath::Max(0.01f, TransitionProfile.ExitAltitudeMultiplier);
	return FMath::Max(ExitAltitude, AdaptiveAlt);
}

float UPlanetSurfaceStreamer::ComputeAutoSurfaceLevelScale() const
{
	constexpr float MinAutoScale = 0.005f;
	const float PlanetRadius = GetPlanetRadiusFromOwner();
	if (PlanetRadius <= 0.f) return 1.f;

	const float SafeExtent = FMath::Max(100.f, SurfaceLevelWorldExtent);
	const float SafePatch = FMath::Clamp(DesiredPatchFraction, 0.01f, 1.f);
	const float DesiredPatchRadius = PlanetRadius * SafePatch;
	return FMath::Clamp(DesiredPatchRadius / SafeExtent, MinAutoScale, 10.f);
}

bool UPlanetSurfaceStreamer::ShouldStreamIn(float DistanceSq) const
{
	const float Radius = GetEffectiveStreamingRadius();
	return DistanceSq <= (Radius * Radius);
}

float UPlanetSurfaceStreamer::GetEffectiveStreamingRadius() const
{
	if (TransitionProfile.bUseExplicitRadiiOverrides && StreamingRadius > 0.f)
	{
		return StreamingRadius;
	}

	const float PlanetRadius = GetPlanetRadiusFromOwner();
	if (PlanetRadius <= 0.f)
	{
		return 200000.f;
	}

	if (!TransitionProfile.bUseAdaptiveRadii)
	{
		return FMath::Max(PlanetRadius * 2.f, 10000.f);
	}

	return ComputeAdaptiveStreamingRadius(PlanetRadius, GetPlayerApproachSpeedTowardPlanet());
}

bool UPlanetSurfaceStreamer::ShouldTransitionToSurface(float DistanceSq) const
{
	const float R = GetEffectiveHandoffRadius();
	return DistanceSq <= (R * R);
}

float UPlanetSurfaceStreamer::GetPlayerApproachSpeedTowardPlanet() const
{
	const AActor* Owner = GetOwner();
	const APawn* PlayerPawn = GetPlayerPawn();
	if (!Owner || !PlayerPawn)
	{
		return 0.f;
	}

	const FVector ToPlanet = (Owner->GetActorLocation() - PlayerPawn->GetActorLocation()).GetSafeNormal();
	if (ToPlanet.IsNearlyZero())
	{
		return 0.f;
	}

	const float TowardSpeed = FVector::DotProduct(PlayerPawn->GetVelocity(), ToPlanet);
	return FMath::Max(0.f, TowardSpeed);
}

float UPlanetSurfaceStreamer::ComputeAdaptiveStreamingRadius(float PlanetRadius, float ApproachSpeed) const
{
	const float SafePlanetRadius = FMath::Max(1.f, PlanetRadius);
	const float SafeApproach = FMath::Clamp(ApproachSpeed, 0.f, FMath::Max(0.f, TransitionProfile.MaxAssumedApproachSpeed));
	const float TimeBudget = FMath::Max(TransitionProfile.LoadLatencyBudgetSeconds, TransitionProfile.MinApproachLeadTimeSeconds);

	const float BaseRadius = SafePlanetRadius * FMath::Max(1.f, TransitionProfile.BaseStreamingRadiusMultiplier);
	const float SpeedMargin = SafeApproach * FMath::Max(0.f, TimeBudget);
	const float Computed = BaseRadius + SpeedMargin;

	const float MinRadius = SafePlanetRadius * FMath::Max(1.f, TransitionProfile.MinStreamingRadiusMultiplier);
	const float MaxRadius = SafePlanetRadius * FMath::Max(TransitionProfile.MinStreamingRadiusMultiplier, TransitionProfile.MaxStreamingRadiusMultiplier);
	return FMath::Clamp(Computed, MinRadius, MaxRadius);
}

float UPlanetSurfaceStreamer::ComputeAdaptiveHandoffRadius(float PlanetRadius, float ApproachSpeed) const
{
	const float SafePlanetRadius = FMath::Max(1.f, PlanetRadius);
	const float SafeApproach = FMath::Clamp(ApproachSpeed, 0.f, FMath::Max(0.f, TransitionProfile.MaxAssumedApproachSpeed));
	const float SettleBudget = FMath::Max(0.f, TransitionProfile.HandoffSettleBudgetSeconds);

	const float BaseRadius = SafePlanetRadius * FMath::Max(1.f, TransitionProfile.BaseHandoffRadiusMultiplier);
	const float SpeedMargin = SafeApproach * SettleBudget;
	const float Computed = BaseRadius + SpeedMargin;

	const float MinRadius = SafePlanetRadius * FMath::Max(1.f, TransitionProfile.MinHandoffRadiusMultiplier);
	const float MaxRadius = SafePlanetRadius * FMath::Max(TransitionProfile.MinHandoffRadiusMultiplier, TransitionProfile.MaxHandoffRadiusMultiplier);
	return FMath::Clamp(Computed, MinRadius, MaxRadius);
}

float UPlanetSurfaceStreamer::GetEffectiveFadeStartRadius() const
{
	if (RevealStartRadiusOverride > 0.f) return RevealStartRadiusOverride;
	const float PlanetRadius = GetPlanetRadiusFromOwner();
	if (PlanetRadius <= 0.f) return 100000.f;
	return PlanetRadius * FadeStartMultiplier;
}

float UPlanetSurfaceStreamer::GetEffectiveFullFadeRadius() const
{
	if (FullFadeRadiusOverride > 0.f) return FullFadeRadiusOverride;
	return GetEffectiveHandoffRadius();
}

float UPlanetSurfaceStreamer::ComputeRevealProgress(float DistanceToPlayer) const
{
	const float FadeStart = GetEffectiveFadeStartRadius();
	const float FullFade = GetEffectiveFullFadeRadius();
	if (FadeStart <= FullFade) return 1.f;

	const float Raw = FMath::Clamp((FadeStart - DistanceToPlayer) / (FadeStart - FullFade), 0.f, 1.f);
	const float SafeExponent = FMath::Max(0.01f, FadeEaseExponent);
	return FMath::Pow(Raw, SafeExponent);
}

void UPlanetSurfaceStreamer::UpdatePlanetFade(float DistanceToPlayer)
{
	CurrentRevealProgress = ComputeRevealProgress(DistanceToPlayer);
	const float Alpha = 1.f - CurrentRevealProgress;
	SetPlanetFadeAlpha(Alpha);
	SetPlanetRevealParams(CurrentRevealProgress);
}

void UPlanetSurfaceStreamer::SetPlanetRevealParams(float Progress)
{
	if (!CachedPlanetFadeMaterial) return;
	AActor* Owner = GetOwner();
	if (!Owner) return;
	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn) return;

	const FVector Center = Owner->GetActorLocation();
	const FVector ToPlayer = (PlayerPawn->GetActorLocation() - Center).GetSafeNormal();
	const float PlanetRadius = FMath::Max(1.f, GetPlanetRadiusFromOwner());

	const FVector RevealCenter = Center + ToPlayer * PlanetRadius;
	const float MinRadius = FMath::Max(10.f, PlanetRadius * FMath::Max(0.f, RevealRadiusMinMultiplier));
	const float MaxRadius = FMath::Max(MinRadius, PlanetRadius * FMath::Max(RevealRadiusMinMultiplier, RevealRadiusMaxMultiplier));
	const float RevealRadius = FMath::Lerp(MinRadius, MaxRadius, Progress);
	const float RevealSoftness = FMath::Max(10.f, PlanetRadius * FMath::Max(0.f, RevealSoftnessMultiplier));

	CachedPlanetFadeMaterial->SetVectorParameterValue(RevealCenterParameterName, FLinearColor(RevealCenter));
	CachedPlanetFadeMaterial->SetScalarParameterValue(RevealRadiusParameterName, RevealRadius);
	CachedPlanetFadeMaterial->SetScalarParameterValue(RevealSoftnessParameterName, RevealSoftness);
}

void UPlanetSurfaceStreamer::ResetPlanetRevealParams()
{
	if (!CachedPlanetFadeMaterial) return;
	// Keep full planet visible outside reveal phase: use a very large sphere mask by default.
	CachedPlanetFadeMaterial->SetVectorParameterValue(RevealCenterParameterName, FLinearColor::Black);
	CachedPlanetFadeMaterial->SetScalarParameterValue(RevealRadiusParameterName, 1.0e9f);
	CachedPlanetFadeMaterial->SetScalarParameterValue(RevealSoftnessParameterName, 1.0e8f);
}

void UPlanetSurfaceStreamer::SetPlanetFadeAlpha(float Alpha)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	UStaticMeshComponent* Mesh = Owner->FindComponentByClass<UStaticMeshComponent>();
	if (!Mesh || Mesh->GetNumMaterials() == 0) return;

	UMaterialInterface* CurrentMat = Mesh->GetMaterial(0);
	if (!CurrentMat) return;

	// Reuse our MID if the mesh is already using it; otherwise we'd create MID(MID(...)) every tick and blow FName length
	if (CachedPlanetFadeMaterial && CurrentMat == CachedPlanetFadeMaterial)
	{
		CachedPlanetFadeMaterial->SetScalarParameterValue(FadeParameterName, Alpha);
		return;
	}

	// Create MID from the base material only (never from an existing MID)
	UMaterialInterface* BaseMat = CurrentMat;
	if (UMaterialInstanceDynamic* ExistingMID = Cast<UMaterialInstanceDynamic>(CurrentMat))
	{
		BaseMat = ExistingMID->Parent;
	}
	if (!BaseMat) return;

	CachedPlanetFadeMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
	if (!CachedPlanetFadeMaterial) return;
	Mesh->SetMaterial(0, CachedPlanetFadeMaterial);
	CachedPlanetFadeMaterial->SetScalarParameterValue(FadeParameterName, Alpha);
}

bool UPlanetSurfaceStreamer::ShouldStreamOut(const FVector& PlayerLocation, const FVector& SurfaceOrigin) const
{
	const FVector Offset = PlayerLocation - SurfaceOrigin;
	const float AltitudeAboveSurface = FVector::DotProduct(Offset, TangentNormal);
	return AltitudeAboveSurface > GetEffectiveExitAltitude();
}

void UPlanetSurfaceStreamer::SaveSpacePosition(const FVector& Location, const FRotator& Rotation)
{
	SavedSpaceLocation = Location;
	SavedSpaceRotation = Rotation;
}

void UPlanetSurfaceStreamer::GetSavedSpacePosition(FVector& OutLocation, FRotator& OutRotation) const
{
	OutLocation = SavedSpaceLocation;
	OutRotation = SavedSpaceRotation;
}

// ---------------------------------------------------------------------------
// State machine updates
// ---------------------------------------------------------------------------

void UPlanetSurfaceStreamer::UpdateIdleState()
{
	APawn* PlayerPawn = GetPlayerPawn();
	if (PlayerPawn)
	{
		if (UPlanetGravityComponent* GravComp = PlayerPawn->FindComponentByClass<UPlanetGravityComponent>())
		{
			GravComp->SetSurfaceBlendAlpha(0.f);
		}
	}

	if (AFederationGameState* GS = GetWorld() ? GetWorld()->GetGameState<AFederationGameState>() : nullptr)
	{
		GS->DebugStreamingState = TEXT("Idle");
		GS->DebugStreamingLevelName = FString();
	}
	const float DistSq = GetDistanceToPlayerSquared();
	if (ShouldStreamIn(DistSq))
	{
		BeginStreamIn();
	}
}

void UPlanetSurfaceStreamer::UpdateLoadingState()
{
	if (!StreamedLevel) return;

	TimeSinceStreamOut += GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;

	const float DistSq = GetDistanceToPlayerSquared();
	const float Dist = FMath::Sqrt(DistSq);
	const float HandoffR = GetEffectiveHandoffRadius();
	const float StreamR = GetEffectiveStreamingRadius();

	APawn* PlayerPawn = GetPlayerPawn();
	if (PlayerPawn)
	{
		if (UPlanetGravityComponent* GravComp = PlayerPawn->FindComponentByClass<UPlanetGravityComponent>())
		{
			float BlendAlpha = 0.f;
			if (StreamR > HandoffR + KINDA_SMALL_NUMBER)
			{
				const float Normalized = (StreamR - Dist) / (StreamR - HandoffR);
				BlendAlpha = FMath::Clamp(Normalized, 0.f, 1.f);
			}
			GravComp->SetSurfaceBlendAlpha(BlendAlpha);
			ApplyAtmosphereRollBlend(BlendAlpha);
		}
	}

	// Only show "Loading/Approaching" in dev HUD when we're close; from far away show Deep Space (Idle)
	if (AFederationGameState* GS = GetWorld() ? GetWorld()->GetGameState<AFederationGameState>() : nullptr)
	{
		if (Dist <= HandoffR * 2.f)
		{
			GS->DebugStreamingState = TEXT("Loading");
			GS->DebugStreamingLevelName = FPaths::GetBaseFilename(SurfaceLevelPath);
		}
		else
		{
			GS->DebugStreamingState = TEXT("Idle");
			GS->DebugStreamingLevelName = FString();
		}
	}

	// If player has moved beyond streaming radius, unload only once the level has finished loading.
	// Do not cancel an in-progress load: let it complete so we can transition if they re-enter range,
	// and avoid the "flash then level never appears" when the player is near the boundary.
	if (!ShouldStreamIn(DistSq))
	{
		if (StreamedLevel->HasLoadedLevel())
		{
			StreamedLevel->SetShouldBeLoaded(false);
			StreamedLevel->SetShouldBeVisible(false);
			StreamedLevel->SetIsRequestingUnloadAndRemoval(true);
			StreamingState = EPlanetStreamingState::Unloading;
			UE_LOG(LogTemp, Log, TEXT("PlanetSurfaceStreamer: Player left streaming range, unloading level."));
		}
		return;
	}

	if (!StreamedLevel->HasLoadedLevel())
	{
		// After hot reload, LoadingStartTime can be 0 on existing instances; avoid immediate false timeout.
		if (GetWorld() && LoadingStartTime <= 0.f)
		{
			LoadingStartTime = GetWorld()->GetTimeSeconds();
		}
		// Timeout: if the level never loads (e.g. bad path), leave Loading and go back to Idle so we can retry.
		const float LoadTimeout = 15.f;
		if (GetWorld() && (GetWorld()->GetTimeSeconds() - LoadingStartTime > LoadTimeout))
		{
			UE_LOG(LogTemp, Warning, TEXT("PlanetSurfaceStreamer: Level load timeout after %.0fs, unloading. Check SurfaceLevelPath='%s'."), LoadTimeout, *SurfaceLevelPath);
			StreamedLevel->SetShouldBeLoaded(false);
			StreamedLevel->SetShouldBeVisible(false);
			StreamedLevel->SetIsRequestingUnloadAndRemoval(true);
			StreamedLevel = nullptr;
			StreamingState = EPlanetStreamingState::Idle;
		}
		return;
	}

	// Avoid flip-flop: require cooldown after stream-out before allowing transition back to surface.
	if (TimeSinceStreamOut < StreamOutReentryCooldownSeconds) return;

	if (!ShouldTransitionToSurface(DistSq)) return;
	const bool bFadeActive = FadeStartMultiplier > 0.f;
	if (bFadeActive && CurrentRevealProgress < HandoffMinRevealProgress) return;
	if (!TryAcquireTransitionLock()) return;

	TransitionPlayerToSurface();
	StreamingState = EPlanetStreamingState::OnSurface;
	if (AFederationGameState* GS = GetWorld() ? GetWorld()->GetGameState<AFederationGameState>() : nullptr)
	{
		GS->DebugStreamingState = TEXT("OnSurface");
		GS->DebugStreamingLevelName = FPaths::GetBaseFilename(SurfaceLevelPath);
	}
	OnSurfaceLoaded.Broadcast();

	UE_LOG(LogTemp, Warning, TEXT("PlanetSurfaceStreamer: === HANDOVER === dist=%.0f, handoff=%.0f, streaming=%.0f, reveal=%.2f"), Dist, HandoffR, StreamR, CurrentRevealProgress);
}

void UPlanetSurfaceStreamer::UpdateOnSurfaceState()
{
	if (AFederationGameState* GS = GetWorld() ? GetWorld()->GetGameState<AFederationGameState>() : nullptr)
	{
		GS->DebugStreamingState = TEXT("OnSurface");
		GS->DebugStreamingLevelName = FPaths::GetBaseFilename(SurfaceLevelPath);
	}
	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn) return;

	if (ShouldStreamOut(PlayerPawn->GetActorLocation(), SurfaceLevelWorldOrigin))
	{
		BeginStreamOut();
	}
}

void UPlanetSurfaceStreamer::UpdateUnloadingState()
{
	if (!StreamedLevel)
	{
		StreamingState = EPlanetStreamingState::Idle;
		return;
	}

	if (!StreamedLevel->HasLoadedLevel())
	{
		ReleaseTransitionLock();
		StreamedLevel = nullptr;
		StreamingState = EPlanetStreamingState::Idle;
		if (AFederationGameState* GS = GetWorld() ? GetWorld()->GetGameState<AFederationGameState>() : nullptr)
		{
			GS->DebugStreamingState = TEXT("Idle");
			GS->DebugStreamingLevelName = FString();
		}
		OnSurfaceUnloaded.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("PlanetSurfaceStreamer: Surface level unloaded, player returned to space."));
	}
}

// ---------------------------------------------------------------------------
// Streaming operations
// ---------------------------------------------------------------------------

void UPlanetSurfaceStreamer::BeginStreamIn()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (SurfaceLevelPath.IsEmpty())
	{
		SurfaceLevelPath = TEXT("/Game/Planets/PlanetSurface_Test");
		UE_LOG(LogTemp, Log, TEXT("PlanetSurfaceStreamer: SurfaceLevelPath was empty, using default '%s'. Set it in Details to use another level."), *SurfaceLevelPath);
	}

	APawn* PlayerPawn = GetPlayerPawn();
	if (PlayerPawn)
	{
		SaveSpacePosition(PlayerPawn->GetActorLocation(), PlayerPawn->GetActorRotation());
	}

	const AActor* Owner = GetOwner();
	const FVector PlanetCenter = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
	const float PlanetRadius = GetPlanetRadiusFromOwner();

	// Place the level at a fixed geographic anchor on the planet surface.
	// The coordinate mapping (SpaceToSurfacePosition) handles routing any
	// approach direction to the correct XY position on this fixed level.
	const FVector AnchorDir = SurfaceAnchorDirection.GetSafeNormal();
	const FVector LevelLoadLocation = (PlanetRadius > 0.f && !AnchorDir.IsNearlyZero())
		? PlanetCenter + AnchorDir * PlanetRadius
		: PlanetCenter;
	SurfaceLevelWorldOrigin = LevelLoadLocation;
	ComputeTangentFrame(PlanetCenter);
	const FRotator LevelRotation = FRotationMatrix::MakeFromXY(TangentX, TangentY).Rotator();
	const float EffectiveScale = (SurfaceLevelScaleMultiplier > 0.f) ? SurfaceLevelScaleMultiplier : ComputeAutoSurfaceLevelScale();
	const FVector Scale3D(EffectiveScale);
	const FTransform LevelTransform(FQuat(LevelRotation), LevelLoadLocation, Scale3D);

	const FSoftObjectPath LevelPath(SurfaceLevelPath);
	TSoftObjectPtr<UWorld> LevelPtr{ LevelPath };
	bool bSuccess = false;
	StreamedLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		this,
		LevelPtr,
		LevelTransform,
		bSuccess
	);

	if (bSuccess && StreamedLevel)
	{
		StreamedLevel->SetShouldBeLoaded(true);
		StreamedLevel->SetShouldBeVisible(true);
		StreamingState = EPlanetStreamingState::Loading;
		TimeSinceStreamOut = StreamOutReentryCooldownSeconds; // Allow immediate transition on first approach.
		LoadingStartTime = World ? World->GetTimeSeconds() : 0.f;
		// Game state (Loading/level name) is set in UpdateLoadingState only when close, so HUD shows "Deep Space" when far
		UE_LOG(LogTemp, Warning, TEXT("PlanetSurfaceStreamer: === STREAM START === level='%s' at surface pos (%.0f, %.0f, %.0f), planet radius=%.0f, scale=%.3f (auto=%s), exitAlt=%.0f"), *SurfaceLevelPath, LevelLoadLocation.X, LevelLoadLocation.Y, LevelLoadLocation.Z, PlanetRadius, EffectiveScale, SurfaceLevelScaleMultiplier <= 0.f ? TEXT("yes") : TEXT("no"), GetEffectiveExitAltitude());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlanetSurfaceStreamer: Failed to begin streaming '%s'. Create a level at Content/Planets/PlanetSurface_Test (save as PlanetSurface_Test) or set SurfaceLevelPath to your level's package name."), *SurfaceLevelPath);
		StreamedLevel = nullptr;
	}
}

void UPlanetSurfaceStreamer::TransitionPlayerToSurface()
{
	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn) return;

	AActor* Owner = GetOwner();
	AFederationCharacter* FedChar = Cast<AFederationCharacter>(PlayerPawn);
	ACharacter* Char = Cast<ACharacter>(PlayerPawn);
	UPlanetGravityComponent* GravComp = PlayerPawn->FindComponentByClass<UPlanetGravityComponent>();
	APlayerController* PC = Char ? Cast<APlayerController>(Char->GetController()) : nullptr;

	// --- 1. Capture full world-space view orientation before any state changes ---
	LastTransitionOrientation = CaptureCurrentViewOrientation();

	// --- 2. Surface origin and tangent frame are fixed at the geographic anchor
	// set during BeginStreamIn. The coordinate mapping handles placing the player
	// at the correct surface position regardless of approach angle.

	// --- 3. Capture incoming velocity ---
	const FVector IncomingVelocity = PlayerPawn->GetVelocity();

	// --- 4. Position player on surface level (altitude mirrors space distance) ---
	PlayerPawn->SetActorLocation(SpaceToSurfacePosition(PlayerPawn->GetActorLocation()));

	// --- 5. Hide planet shell (should already be nearly invisible from fade) ---
	if (Owner)
	{
		Owner->SetActorHiddenInGame(true);
		Owner->SetActorEnableCollision(false);
	}

	// --- 6. Switch to surface gravity + preserve velocity ---
	const FVector SurfaceUp = TangentNormal;
	const FVector SurfaceDown = -TangentNormal;

	if (Char && Char->GetCharacterMovement())
	{
		Char->GetCharacterMovement()->SetGravityDirection(SurfaceDown);
		Char->GetCharacterMovement()->GravityScale = 1.f;
		const float DownSpeed = FMath::Max(0.f, -FVector::DotProduct(IncomingVelocity, SurfaceUp));
		const FVector TangentVelocity = IncomingVelocity - FVector::DotProduct(IncomingVelocity, SurfaceUp) * SurfaceUp;
		Char->GetCharacterMovement()->Velocity = TangentVelocity + SurfaceDown * DownSpeed;
	}

	// --- 7. Preserve forward exactly, blend roll to local horizon at handoff ---
	FQuat SurfaceViewQuat = LastTransitionOrientation.bIsValid
		? ComputeForwardPriorityBlendedViewQuat(LastTransitionOrientation.ViewQuat, SurfaceUp, 1.f)
		: FQuat::Identity;

	if (!LastTransitionOrientation.bIsValid && PC)
	{
		SurfaceViewQuat = PC->GetControlRotation().Quaternion();
	}

	const FRotator SurfaceControlRotation = SurfaceViewQuat.Rotator();
	if (FedChar && FedChar->FirstPersonCameraRoot)
	{
		FedChar->FirstPersonCameraRoot->SetWorldRotation(SurfaceViewQuat);
		FedChar->FirstPersonCameraRoot->SetRelativeRotation(FRotator::ZeroRotator);
	}

	if (Char)
	{
		Char->bUseControllerRotationYaw = true;
		Char->bUseControllerRotationPitch = true;
		Char->bUseControllerRotationRoll = true;
		Char->SetActorRotation(SurfaceViewQuat);
	}
	if (PC)
	{
		PC->SetControlRotation(SurfaceControlRotation);
	}

	// --- 8. Reset gravity component to surface-aligned state ---
	if (GravComp)
	{
		GravComp->GravityDir = SurfaceDown;
		GravComp->bViewInitialized = false;
		GravComp->SetSurfaceBlendAlpha(1.f);
	}
	SetPlayerGravityComponentActive(false);
}

void UPlanetSurfaceStreamer::BeginStreamOut()
{
	TransitionPlayerToSpace();

	TimeSinceStreamOut = 0.f;

	// Keep the level loaded and visible so the player can see it in deep space
	// as they fly away. It will be unloaded when they leave streaming range.
	StreamingState = EPlanetStreamingState::Loading;
	UE_LOG(LogTemp, Warning, TEXT("PlanetSurfaceStreamer: === EXIT TO SPACE === level kept visible, state=Loading"));
}

void UPlanetSurfaceStreamer::TransitionPlayerToSpace()
{
	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn) return;

	AActor* Owner = GetOwner();
	ACharacter* Char = Cast<ACharacter>(PlayerPawn);
	APlayerController* PC = Char ? Cast<APlayerController>(Char->GetController()) : nullptr;

	// --- 1. Capture current full view orientation before any state changes ---
	LastTransitionOrientation = CaptureCurrentViewOrientation();

	// --- 2. Show the planet sphere again ---
	if (Owner)
	{
		Owner->SetActorHiddenInGame(false);
		Owner->SetActorEnableCollision(true);
	}
	SetPlanetFadeAlpha(1.f);
	ResetPlanetRevealParams();
	CurrentRevealProgress = 0.f;

	// --- 3. Place player at matching distance from planet surface ---
	PlayerPawn->SetActorLocation(SurfaceToSpacePosition(PlayerPawn->GetActorLocation()));

	// --- 4. Restore gravity-relative look with preserved forward direction ---
	if (Char)
	{
		const FQuat RestoredViewQuat = LastTransitionOrientation.bIsValid ? LastTransitionOrientation.ViewQuat : Char->GetActorQuat();
		Char->SetActorRotation(RestoredViewQuat);
		Char->bUseControllerRotationYaw = false;
		Char->bUseControllerRotationPitch = false;
		Char->bUseControllerRotationRoll = false;
	}

	SetPlayerGravityComponentActive(true);

	if (UPlanetGravityComponent* GravComp = PlayerPawn->FindComponentByClass<UPlanetGravityComponent>())
	{
		GravComp->SetSurfaceBlendAlpha(0.f);

		const FVector GravUp = GravComp->GetGravityUp();
		const FVector PreservedForward = LastTransitionOrientation.bIsValid
			? LastTransitionOrientation.Forward
			: (PC ? PC->GetControlRotation().Vector() : PlayerPawn->GetActorForwardVector());
		FVector TangentFwd = (PreservedForward - FVector::DotProduct(PreservedForward, GravUp) * GravUp).GetSafeNormal();
		if (TangentFwd.IsNearlyZero())
		{
			TangentFwd = FVector::CrossProduct(GravUp, FVector::RightVector).GetSafeNormal();
		}
		GravComp->ViewTangentForward = TangentFwd;
		GravComp->ViewUp = GravUp;
		GravComp->ViewPitchRad = FMath::Asin(FMath::Clamp(FVector::DotProduct(PreservedForward.GetSafeNormal(), GravUp), -1.f, 1.f));
		GravComp->bViewInitialized = true;
	}

	if (PC && LastTransitionOrientation.bIsValid)
	{
		PC->SetControlRotation(LastTransitionOrientation.ViewQuat.Rotator());
	}
	ReleaseTransitionLock();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void UPlanetSurfaceStreamer::ComputeTangentFrame(const FVector& PlanetCenter)
{
	TangentNormal = (SurfaceLevelWorldOrigin - PlanetCenter).GetSafeNormal();
	if (TangentNormal.IsNearlyZero())
	{
		TangentNormal = FVector::UpVector;
	}

	const FVector UpHint = FMath::Abs(TangentNormal.Z) < 0.99f ? FVector::UpVector : FVector::ForwardVector;
	TangentX = FVector::CrossProduct(UpHint, TangentNormal).GetSafeNormal();
	TangentY = FVector::CrossProduct(TangentNormal, TangentX).GetSafeNormal();
}

FVector UPlanetSurfaceStreamer::SpaceToSurfacePosition(const FVector& SpacePos) const
{
	const AActor* Owner = GetOwner();
	const FVector PlanetCenter = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
	const float R = FMath::Max(1.f, GetPlanetRadiusFromOwner());

	const FVector DirToPlayer = (SpacePos - PlanetCenter).GetSafeNormal();
	const float Altitude = FVector::Dist(SpacePos, PlanetCenter) - R;

	const float DotN = FVector::DotProduct(DirToPlayer, TangentNormal);
	const float DotX = FVector::DotProduct(DirToPlayer, TangentX);
	const float DotY = FVector::DotProduct(DirToPlayer, TangentY);

	const float Azimuth = FMath::Atan2(DotX, DotN);
	const float Elevation = FMath::Atan2(DotY, DotN);

	const float LocalX = Azimuth * R;
	const float LocalY = Elevation * R;
	const float LocalZ = FMath::Max(300.f, Altitude);

	return SurfaceLevelWorldOrigin + TangentX * LocalX + TangentY * LocalY + TangentNormal * LocalZ;
}

FVector UPlanetSurfaceStreamer::SurfaceToSpacePosition(const FVector& SurfacePos) const
{
	const AActor* Owner = GetOwner();
	const FVector PlanetCenter = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
	const float R = FMath::Max(1.f, GetPlanetRadiusFromOwner());
	const FVector Offset = SurfacePos - SurfaceLevelWorldOrigin;

	const float LocalX = FVector::DotProduct(Offset, TangentX);
	const float LocalY = FVector::DotProduct(Offset, TangentY);
	const float Altitude = FVector::DotProduct(Offset, TangentNormal);

	const float AngleX = LocalX / R;
	const float AngleY = LocalY / R;

	// Rodrigues rotation: rotate TangentNormal around TangentY by AngleX,
	// then around TangentX by -AngleY to get the sphere direction.
	FVector Dir = TangentNormal;
	Dir = Dir.RotateAngleAxis(FMath::RadiansToDegrees(AngleX), TangentY);
	Dir = Dir.RotateAngleAxis(-FMath::RadiansToDegrees(AngleY), TangentX);
	Dir = Dir.GetSafeNormal();

	return PlanetCenter + Dir * (R + FMath::Max(0.f, Altitude));
}

FPlanetTransitionOrientation UPlanetSurfaceStreamer::CaptureCurrentViewOrientation() const
{
	FPlanetTransitionOrientation Orientation;

	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn)
	{
		return Orientation;
	}

	FQuat ViewQuat = PlayerPawn->GetActorQuat();
	if (const AFederationCharacter* FedChar = Cast<AFederationCharacter>(PlayerPawn))
	{
		if (FedChar->FirstPersonCameraRoot)
		{
			ViewQuat = FedChar->FirstPersonCameraRoot->GetComponentQuat();
		}
		else if (const APlayerController* PC = Cast<APlayerController>(FedChar->GetController()))
		{
			ViewQuat = PC->GetControlRotation().Quaternion();
		}
	}
	else if (const APlayerController* PC = Cast<APlayerController>(PlayerPawn->GetController()))
	{
		ViewQuat = PC->GetControlRotation().Quaternion();
	}

	Orientation.ViewQuat = ViewQuat;
	Orientation.Forward = ViewQuat.GetForwardVector().GetSafeNormal();
	Orientation.Up = ViewQuat.GetUpVector().GetSafeNormal();
	Orientation.bIsValid = !Orientation.Forward.IsNearlyZero() && !Orientation.Up.IsNearlyZero();
	return Orientation;
}

FQuat UPlanetSurfaceStreamer::ComputeForwardPriorityBlendedViewQuat(const FQuat& CurrentView, const FVector& TargetUp, float BlendAlpha) const
{
	const float Alpha = FMath::Clamp(BlendAlpha, 0.f, 1.f);
	const FVector Forward = CurrentView.GetForwardVector().GetSafeNormal();
	if (Forward.IsNearlyZero())
	{
		return CurrentView;
	}

	FVector CurrentUp = CurrentView.GetUpVector().GetSafeNormal();
	FVector SourceUp = (CurrentUp - FVector::DotProduct(CurrentUp, Forward) * Forward).GetSafeNormal();
	if (SourceUp.IsNearlyZero())
	{
		SourceUp = FVector::UpVector;
		if (FMath::Abs(FVector::DotProduct(SourceUp, Forward)) > 0.95f)
		{
			SourceUp = FVector::RightVector;
		}
		SourceUp = (SourceUp - FVector::DotProduct(SourceUp, Forward) * Forward).GetSafeNormal();
	}

	FVector DesiredUp = (TargetUp.GetSafeNormal() - FVector::DotProduct(TargetUp.GetSafeNormal(), Forward) * Forward).GetSafeNormal();
	if (DesiredUp.IsNearlyZero())
	{
		DesiredUp = SourceUp;
	}

	const FQuat Delta = FQuat::FindBetweenNormals(SourceUp, DesiredUp);
	const FQuat RollBlendQuat = FQuat::Slerp(FQuat::Identity, Delta, Alpha);
	const FVector BlendedUp = RollBlendQuat.RotateVector(SourceUp).GetSafeNormal();

	return FRotationMatrix::MakeFromXZ(Forward, BlendedUp).ToQuat();
}

void UPlanetSurfaceStreamer::ApplyAtmosphereRollBlend(float BlendAlpha)
{
	APawn* PlayerPawn = GetPlayerPawn();
	AFederationCharacter* FedChar = Cast<AFederationCharacter>(PlayerPawn);
	APlayerController* PC = FedChar ? Cast<APlayerController>(FedChar->GetController()) : nullptr;
	if (!FedChar || !PC || !FedChar->JetpackComponent || !FedChar->JetpackComponent->IsJetpackEnabled())
	{
		return;
	}

	UPlanetGravityComponent* GravComp = FedChar->FindComponentByClass<UPlanetGravityComponent>();
	if (!GravComp)
	{
		return;
	}

	const FQuat CurrentViewQuat = FedChar->FirstPersonCameraRoot
		? FedChar->FirstPersonCameraRoot->GetComponentQuat()
		: PC->GetControlRotation().Quaternion();
	const FQuat BlendedQuat = ComputeForwardPriorityBlendedViewQuat(CurrentViewQuat, GravComp->GetGravityUp(), BlendAlpha);
	PC->SetControlRotation(BlendedQuat.Rotator());
}

APawn* UPlanetSurfaceStreamer::GetPlayerPawn() const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	return UGameplayStatics::GetPlayerPawn(World, 0);
}

void UPlanetSurfaceStreamer::SetPlayerGravityComponentActive(bool bActive)
{
	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn) return;

	UPlanetGravityComponent* GravComp = PlayerPawn->FindComponentByClass<UPlanetGravityComponent>();
	if (GravComp)
	{
		GravComp->SetComponentTickEnabled(bActive);
		if (bActive)
		{
			GravComp->SetSurfaceBlendAlpha(0.f);
		}
	}
}


bool UPlanetSurfaceStreamer::TryAcquireTransitionLock()
{
	if (GPlanetTransitionOwner.IsValid() && GPlanetTransitionOwner.Get() != this)
	{
		return false;
	}
	GPlanetTransitionOwner = this;
	bOwnsTransitionLock = true;
	return true;
}

void UPlanetSurfaceStreamer::ReleaseTransitionLock()
{
	if (bOwnsTransitionLock && GPlanetTransitionOwner.Get() == this)
	{
		GPlanetTransitionOwner.Reset();
	}
	bOwnsTransitionLock = false;
}

