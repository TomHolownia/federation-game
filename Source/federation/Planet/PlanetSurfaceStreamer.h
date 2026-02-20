// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlanetSurfaceStreamer.generated.h"

class ULevelStreamingDynamic;
class UPlanetGravityComponent;

UENUM(BlueprintType)
enum class EPlanetStreamingState : uint8
{
	Idle,
	Loading,
	OnSurface,
	Unloading
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSurfaceStreamingEvent);

/**
 * Streams a separate planet surface level when the player approaches
 * a planet sphere actor. Manages the full lifecycle: proximity detection,
 * level streaming, player teleportation, and gravity mode switching.
 *
 * Attach to a planet sphere actor in the space level. Set SurfaceLevelPath
 * to the long package name of the planet surface level (e.g.
 * "/Game/Planets/PlanetSurface_Test").
 */
UCLASS(ClassGroup = "Federation", meta = (BlueprintSpawnableComponent))
class FEDERATION_API UPlanetSurfaceStreamer : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlanetSurfaceStreamer();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Configuration ---

	/** Long package name of the surface level to stream, e.g. "/Game/Planets/PlanetSurface_Test". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
	FString SurfaceLevelPath;

	/** Distance from planet center at which streaming begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
	float StreamingRadius = 200000.f;

	/** Where to place the player relative to the loaded level origin. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
	FVector SurfaceSpawnOffset = FVector(0.f, 0.f, 500.f);

	/** Player altitude above surface-level origin that triggers exit back to space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
	float ExitAltitude = 50000.f;

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Streaming")
	FOnSurfaceStreamingEvent OnSurfaceLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Streaming")
	FOnSurfaceStreamingEvent OnSurfaceUnloaded;

	// --- Public API ---

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	EPlanetStreamingState GetStreamingState() const { return StreamingState; }

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	bool IsPlayerOnSurface() const { return StreamingState == EPlanetStreamingState::OnSurface; }

	// --- Testable logic (public so tests can call directly) ---

	/** Squared distance from the owning actor to the first player pawn. Returns FLT_MAX if unavailable. */
	float GetDistanceToPlayerSquared() const;

	/** True if the given squared distance is within StreamingRadius. */
	bool ShouldStreamIn(float DistanceSq) const;

	/** True if the player's Z (relative to surface origin) exceeds ExitAltitude. */
	bool ShouldStreamOut(const FVector& PlayerLocation, const FVector& SurfaceOrigin) const;

	void SaveSpacePosition(const FVector& Location, const FRotator& Rotation);
	void GetSavedSpacePosition(FVector& OutLocation, FRotator& OutRotation) const;

	/** Transition state (exposed for testing; normal flow drives this via Tick). */
	void SetStreamingState(EPlanetStreamingState NewState) { StreamingState = NewState; }

	// --- State (public for testing) ---

	EPlanetStreamingState StreamingState = EPlanetStreamingState::Idle;
	FVector SavedSpaceLocation = FVector::ZeroVector;
	FRotator SavedSpaceRotation = FRotator::ZeroRotator;

private:
	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> StreamedLevel;

	void UpdateIdleState();
	void UpdateLoadingState();
	void UpdateOnSurfaceState();
	void UpdateUnloadingState();

	void BeginStreamIn();
	void TransitionPlayerToSurface();
	void BeginStreamOut();
	void TransitionPlayerToSpace();

	APawn* GetPlayerPawn() const;
	void SetPlayerGravityComponentActive(bool bActive);
};
