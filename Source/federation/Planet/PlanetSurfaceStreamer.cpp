// Copyright Federation Game. All Rights Reserved.

#include "Planet/PlanetSurfaceStreamer.h"
#include "Planet/PlanetGravityComponent.h"
#include "Character/FederationCharacter.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"

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
	if (TransitionProfile.bUseExplicitRadiiOverrides && HandoffRadius > 0.f) return HandoffRadius;

	if (TransitionProfile.bUseAdaptiveRadii)
	{
		const float PlanetRadius = GetPlanetRadiusFromOwner();
		if (PlanetRadius > 0.f)
		{
			return ComputeAdaptiveHandoffRadius(PlanetRadius, GetPlayerApproachSpeedTowardPlanet());
		}
	}

	const float PlanetRadius = GetPlanetRadiusFromOwner();
	if (PlanetRadius <= 0.f) return 100000.f;
	// Add margin so we trigger when at the surface (player center can be slightly outside radius when standing on the sphere).
	const float Margin = FMath::Max(500.f, PlanetRadius * 0.05f);
	return PlanetRadius + Margin;
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
	const float AltitudeAboveSurface = (PlayerLocation - SurfaceOrigin).Size();
	return AltitudeAboveSurface > ExitAltitude;
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
	const float DistSq = GetDistanceToPlayerSquared();
	if (ShouldStreamIn(DistSq))
	{
		BeginStreamIn();
	}
}

void UPlanetSurfaceStreamer::UpdateLoadingState()
{
	if (!StreamedLevel) return;
	if (!StreamedLevel->HasLoadedLevel()) return;

	const float DistSq = GetDistanceToPlayerSquared();
	if (!ShouldTransitionToSurface(DistSq)) return;
	const bool bFadeActive = FadeStartMultiplier > 0.f;
	if (bFadeActive && CurrentRevealProgress < HandoffMinRevealProgress) return;
	if (!TryAcquireTransitionLock()) return;

	TransitionPlayerToSurface();
	StreamingState = EPlanetStreamingState::OnSurface;
	OnSurfaceLoaded.Broadcast();

	const float Dist = FMath::Sqrt(DistSq);
	const float HandoffR = GetEffectiveHandoffRadius();
	const float StreamR = GetEffectiveStreamingRadius();
	UE_LOG(LogTemp, Warning, TEXT("PlanetSurfaceStreamer: === HANDOVER === dist=%.0f, handoff=%.0f, streaming=%.0f, reveal=%.2f"), Dist, HandoffR, StreamR, CurrentRevealProgress);
}

void UPlanetSurfaceStreamer::UpdateOnSurfaceState()
{
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

	// Place the level at the planet's surface facing the player so the terrain
	// renders IN FRONT of the planet sphere during approach.
	FVector LevelLoadLocation = PlanetCenter;
	if (PlayerPawn && PlanetRadius > 0.f)
	{
		const FVector ToPlayer = (PlayerPawn->GetActorLocation() - PlanetCenter).GetSafeNormal();
		LevelLoadLocation = PlanetCenter + ToPlayer * PlanetRadius;
	}
	SurfaceLevelWorldOrigin = LevelLoadLocation;

	bool bSuccess = false;
	StreamedLevel = ULevelStreamingDynamic::LoadLevelInstance(
		World,
		SurfaceLevelPath,
		LevelLoadLocation,
		FRotator::ZeroRotator,
		bSuccess
	);

	if (bSuccess && StreamedLevel)
	{
		StreamedLevel->SetShouldBeLoaded(true);
		StreamedLevel->SetShouldBeVisible(true);
		StreamingState = EPlanetStreamingState::Loading;
		UE_LOG(LogTemp, Warning, TEXT("PlanetSurfaceStreamer: === STREAM START === level='%s' at surface pos (%.0f, %.0f, %.0f), planet radius=%.0f"), *SurfaceLevelPath, LevelLoadLocation.X, LevelLoadLocation.Y, LevelLoadLocation.Z, PlanetRadius);
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

	// --- 1. Capture current look direction before any state changes ---
	FRotator PreservedLook = FRotator::ZeroRotator;
	if (GravComp && GravComp->IsGravityViewInitialized())
	{
		const FVector Up = GravComp->GetGravityUp();
		const FVector Fwd = GravComp->GetViewTangentForward();
		const float Pitch = FMath::RadiansToDegrees(GravComp->GetViewPitchRad());
		const float Yaw = FMath::Atan2(Fwd.Y, Fwd.X) * (180.f / PI);
		PreservedLook = FRotator(FMath::ClampAngle(Pitch, -85.f, 85.f), Yaw, 0.f);
	}
	else if (PC)
	{
		const FRotator CR = PC->GetControlRotation();
		PreservedLook = FRotator(FMath::ClampAngle(CR.Pitch, -85.f, 85.f), CR.Yaw, 0.f);
	}

	// --- 2. Capture incoming velocity ---
	const FVector IncomingVelocity = PlayerPawn->GetVelocity();

	// --- 3. Position player on surface level (loaded at planet surface facing player) ---
	// Use actual distance from the surface level origin so the perceived height matches.
	const float DistToSurface = FVector::Dist(PlayerPawn->GetActorLocation(), SurfaceLevelWorldOrigin);
	const float EntryAltitude = FMath::Max(300.f, DistToSurface);
	PlayerPawn->SetActorLocation(SurfaceLevelWorldOrigin + FVector(SurfaceSpawnOffset.X, SurfaceSpawnOffset.Y, EntryAltitude));
	if (Char)
	{
		Char->SetActorRotation(FRotator(0.f, PreservedLook.Yaw, 0.f));
	}

	// --- 4. Hide planet shell (should already be nearly invisible from fade) ---
	if (Owner)
	{
		Owner->SetActorHiddenInGame(true);
		Owner->SetActorEnableCollision(false);
	}

	// --- 5. Switch to flat gravity + preserve velocity ---
	if (Char && Char->GetCharacterMovement())
	{
		Char->GetCharacterMovement()->SetGravityDirection(FVector::DownVector);
		const float DownSpeed = FMath::Max(0.f, -IncomingVelocity.Z);
		Char->GetCharacterMovement()->Velocity = FVector(IncomingVelocity.X, IncomingVelocity.Y, -DownSpeed);
	}

	// --- 6. Set camera to preserved look direction ---
	if (FedChar && FedChar->FirstPersonCameraRoot)
	{
		FedChar->FirstPersonCameraRoot->SetWorldRotation(PreservedLook);
		FedChar->FirstPersonCameraRoot->SetRelativeRotation(FRotator::ZeroRotator);
	}

	if (Char)
	{
		Char->bUseControllerRotationYaw = true;
		Char->bUseControllerRotationPitch = true;
	}
	if (PC)
	{
		PC->SetControlRotation(PreservedLook);
	}

	// --- 7. Reset gravity component to flat-world state ---
	if (GravComp)
	{
		GravComp->GravityDir = FVector::DownVector;
		GravComp->bViewInitialized = false;
		GravComp->SetSurfaceBlendAlpha(1.f);
	}
	SetPlayerGravityComponentActive(false);
}

void UPlanetSurfaceStreamer::BeginStreamOut()
{
	TransitionPlayerToSpace();

	if (StreamedLevel)
	{
		StreamedLevel->SetShouldBeLoaded(false);
		StreamedLevel->SetShouldBeVisible(false);
		StreamedLevel->SetIsRequestingUnloadAndRemoval(true);
	}

	StreamingState = EPlanetStreamingState::Unloading;
	UE_LOG(LogTemp, Log, TEXT("PlanetSurfaceStreamer: Began streaming out surface level."));
}

void UPlanetSurfaceStreamer::TransitionPlayerToSpace()
{
	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn) return;

	// Show the planet sphere again and reset fade so it's opaque next time
	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->SetActorHiddenInGame(false);
		Owner->SetActorEnableCollision(true);
	}
	SetPlanetFadeAlpha(1.f);
	ResetPlanetRevealParams();
	CurrentRevealProgress = 0.f;

	PlayerPawn->SetActorLocation(SavedSpaceLocation);
	PlayerPawn->SetActorRotation(SavedSpaceRotation);

	// Restore gravity-relative look (disable controller rotation, gravity comp handles it)
	ACharacter* Char = Cast<ACharacter>(PlayerPawn);
	if (Char)
	{
		Char->bUseControllerRotationYaw = false;
		Char->bUseControllerRotationPitch = false;
	}

	SetPlayerGravityComponentActive(true);

	if (UPlanetGravityComponent* GravComp = PlayerPawn->FindComponentByClass<UPlanetGravityComponent>())
	{
		GravComp->SetSurfaceBlendAlpha(0.f);
	}
	ReleaseTransitionLock();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

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

