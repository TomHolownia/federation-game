// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlanetSurfaceStreamer.generated.h"

class ULevelStreamingDynamic;
class UPlanetGravityComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

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

	/** Distance from planet center at which streaming begins. Should be greater than the planet's visual radius (e.g. SmallPlanet PlanetRadius 100000 → sphere radius 100000; use StreamingRadius 200000 so load starts while approaching). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
	float StreamingRadius = 200000.f;

	/** Distance from planet center at which we teleport to the surface level. If 0 (default), we use the planet actor's bounding radius so it matches your actual planet size. Set a value only to override (e.g. for testing). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
	float HandoffRadius = 0.f;

	/** Where to place the player relative to the loaded level origin. Spawn high so player falls onto terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
	FVector SurfaceSpawnOffset = FVector(0.f, 0.f, 10000.f);

	/** Player altitude above surface-level origin that triggers exit back to space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
	float ExitAltitude = 50000.f;

	/** Planet sphere fade: multiplier of planet radius at which fade starts (e.g. 1.2 = start fading when within 1.2x radius). Fade reaches 0 at handoff. Set to 0 to disable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	float FadeStartMultiplier = 1.2f;

	/** Optional absolute radius where fade starts. If > 0, overrides FadeStartMultiplier-derived distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	float RevealStartRadiusOverride = 0.f;

	/** Optional absolute radius where fade reaches fully hidden. If > 0, overrides handoff-derived distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	float FullFadeRadiusOverride = 0.f;

	/** Progress easing exponent for fade/reveal (1 = linear, >1 = ease-in, <1 = ease-out). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	float FadeEaseExponent = 1.f;

	/** Minimum/maximum reveal patch radius as multipliers of planet radius. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	float RevealRadiusMinMultiplier = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	float RevealRadiusMaxMultiplier = 0.35f;

	/** Soft edge size for reveal patch as multiplier of planet radius. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	float RevealSoftnessMultiplier = 0.08f;

	/** Require reveal progress before teleport handoff can occur. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	float HandoffMinRevealProgress = 0.9f;

	/** Material scalar parameter name on the planet mesh that drives opacity (0 = invisible, 1 = opaque). Add this parameter to your planet material and multiply opacity by it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	FName FadeParameterName = TEXT("FadeAlpha");

	/** Optional material params for directional front reveal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	FName RevealCenterParameterName = TEXT("RevealCenterWS");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	FName RevealRadiusParameterName = TEXT("RevealRadius");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	FName RevealSoftnessParameterName = TEXT("RevealSoftness");

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

	/** True if the given squared distance is within HandoffRadius (player at surface; safe to teleport). */
	bool ShouldTransitionToSurface(float DistanceSq) const;

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

	/** World-space radius of the owner (planet) from its bounding box. 0 if no owner or no bounds. */
	float GetPlanetRadiusFromOwner() const;
	/** Handoff radius to use: HandoffRadius if > 0, else planet bounds radius (with fallback). */
	float GetEffectiveHandoffRadius() const;

	/** Radius at which planet fade starts (FadeStartMultiplier * planet radius). */
	float GetEffectiveFadeStartRadius() const;
	float GetEffectiveFullFadeRadius() const;
	float ComputeRevealProgress(float DistanceToPlayer) const;

	void UpdatePlanetFade(float DistanceToPlayer);
	void SetPlanetFadeAlpha(float Alpha);
	void SetPlanetRevealParams(float Progress);
	void ResetPlanetRevealParams();

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CachedPlanetFadeMaterial;

	float CurrentRevealProgress = 0.f;
};
