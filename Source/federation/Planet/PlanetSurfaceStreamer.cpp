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
	if (HandoffRadius > 0.f) return HandoffRadius;
	const float PlanetRadius = GetPlanetRadiusFromOwner();
	if (PlanetRadius <= 0.f) return 100000.f;
	// Add margin so we trigger when at the surface (player center can be slightly outside radius when standing on the sphere).
	const float Margin = FMath::Max(500.f, PlanetRadius * 0.05f);
	return PlanetRadius + Margin;
}

bool UPlanetSurfaceStreamer::ShouldStreamIn(float DistanceSq) const
{
	return DistanceSq <= (StreamingRadius * StreamingRadius);
}

bool UPlanetSurfaceStreamer::ShouldTransitionToSurface(float DistanceSq) const
{
	const float R = GetEffectiveHandoffRadius();
	return DistanceSq <= (R * R);
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

	if (StreamedLevel->HasLoadedLevel())
	{
		const float DistSq = GetDistanceToPlayerSquared();
		const float Dist = FMath::Sqrt(DistSq);
		const float HandoffR = GetEffectiveHandoffRadius();
		if (ShouldTransitionToSurface(DistSq) && CurrentRevealProgress >= HandoffMinRevealProgress)
		{
			TransitionPlayerToSurface();
			StreamingState = EPlanetStreamingState::OnSurface;
			OnSurfaceLoaded.Broadcast();
			UE_LOG(LogTemp, Log, TEXT("PlanetSurfaceStreamer: Surface level loaded, player at surface (dist=%.0f, handoff=%.0f, reveal=%.2f) — transitioned to surface."), Dist, HandoffR, CurrentRevealProgress);
		}
	}
}

void UPlanetSurfaceStreamer::UpdateOnSurfaceState()
{
	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn) return;

	AActor* Owner = GetOwner();
	const FVector SurfaceOrigin = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;

	if (ShouldStreamOut(PlayerPawn->GetActorLocation(), SurfaceOrigin))
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

	bool bSuccess = false;
	StreamedLevel = ULevelStreamingDynamic::LoadLevelInstance(
		World,
		SurfaceLevelPath,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		bSuccess
	);

	if (bSuccess && StreamedLevel)
	{
		StreamedLevel->SetShouldBeLoaded(true);
		StreamedLevel->SetShouldBeVisible(true);  // Must be visible for level to complete loading and transition to work; surface sky may show briefly until we land
		StreamingState = EPlanetStreamingState::Loading;
		UE_LOG(LogTemp, Log, TEXT("PlanetSurfaceStreamer: Began streaming in '%s'."), *SurfaceLevelPath);
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

	// Hide the planet sphere so it doesn't appear on the surface
	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->SetActorHiddenInGame(true);
		Owner->SetActorEnableCollision(false);
	}

	PlayerPawn->SetActorLocation(SurfaceSpawnOffset);
	PlayerPawn->SetActorRotation(FRotator::ZeroRotator);

	SetPlayerGravityComponentActive(false);

	// Reset gravity component state so any remaining reads are consistent with flat world.
	UPlanetGravityComponent* GravComp = PlayerPawn->FindComponentByClass<UPlanetGravityComponent>();
	if (GravComp)
	{
		GravComp->GravityDir = FVector::DownVector;
		GravComp->bViewInitialized = false;
	}

	AFederationCharacter* FedChar = Cast<AFederationCharacter>(PlayerPawn);
	ACharacter* Char = Cast<ACharacter>(PlayerPawn);
	if (Char)
	{
		if (Char->GetCharacterMovement())
		{
			Char->GetCharacterMovement()->SetGravityDirection(FVector::DownVector);
		}

		// Reset camera root rotation so the view isn't stuck in the last gravity-relative orientation.
		// FirstPersonCameraRoot uses absolute rotation, so we need to reset it explicitly.
		if (FedChar && FedChar->FirstPersonCameraRoot)
		{
			FedChar->FirstPersonCameraRoot->SetWorldRotation(FRotator::ZeroRotator);
			// Reset relative rotation too, in case it was offset
			FedChar->FirstPersonCameraRoot->SetRelativeRotation(FRotator::ZeroRotator);
		}

		// Re-enable controller rotation so standard mouse look works on the surface
		Char->bUseControllerRotationYaw = true;
		Char->bUseControllerRotationPitch = true;
		
		// Reset control rotation to match actor rotation
		if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
		{
			PC->SetControlRotation(FRotator::ZeroRotator);
		}
	}
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
	}
}
