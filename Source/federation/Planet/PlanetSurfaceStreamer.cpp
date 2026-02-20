// Copyright Federation Game. All Rights Reserved.

#include "Planet/PlanetSurfaceStreamer.h"
#include "Planet/PlanetGravityComponent.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UPlanetSurfaceStreamer::UPlanetSurfaceStreamer()
{
	PrimaryComponentTick.bCanEverTick = true;
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

bool UPlanetSurfaceStreamer::ShouldStreamIn(float DistanceSq) const
{
	return DistanceSq <= (StreamingRadius * StreamingRadius);
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
		TransitionPlayerToSurface();
		StreamingState = EPlanetStreamingState::OnSurface;
		OnSurfaceLoaded.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("PlanetSurfaceStreamer: Surface level loaded, player transitioned to surface."));
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
	if (!World || SurfaceLevelPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlanetSurfaceStreamer: Cannot stream in — no world or empty SurfaceLevelPath."));
		return;
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
		StreamedLevel->SetShouldBeVisible(true);
		StreamingState = EPlanetStreamingState::Loading;
		UE_LOG(LogTemp, Log, TEXT("PlanetSurfaceStreamer: Began streaming in '%s'."), *SurfaceLevelPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlanetSurfaceStreamer: Failed to begin streaming '%s'."), *SurfaceLevelPath);
		StreamedLevel = nullptr;
	}
}

void UPlanetSurfaceStreamer::TransitionPlayerToSurface()
{
	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn) return;

	PlayerPawn->SetActorLocation(SurfaceSpawnOffset);
	PlayerPawn->SetActorRotation(FRotator::ZeroRotator);

	SetPlayerGravityComponentActive(false);

	ACharacter* Char = Cast<ACharacter>(PlayerPawn);
	if (Char && Char->GetCharacterMovement())
	{
		Char->GetCharacterMovement()->SetGravityDirection(FVector::DownVector);
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

	PlayerPawn->SetActorLocation(SavedSpaceLocation);
	PlayerPawn->SetActorRotation(SavedSpaceRotation);

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
