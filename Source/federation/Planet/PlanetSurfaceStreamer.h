// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlanetSurfaceStreamer.generated.h"

class ULevelStreamingDynamic;
class UPlanetGravityComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class USceneComponent;

USTRUCT(BlueprintType)
struct FPlanetTransitionOrientation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Transition")
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "Transition")
	FQuat ViewQuat = FQuat::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "Transition")
	FVector Forward = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category = "Transition")
	FVector Up = FVector::UpVector;
};

UENUM(BlueprintType)
enum class EPlanetStreamingState : uint8
{
	Idle,
	Loading,
	OnSurface,
	Unloading
};

UENUM(BlueprintType)
enum class EPlanetTransitionMode : uint8
{
	LegacyHandover UMETA(DisplayName = "Legacy Handover"),
	BlendedAligned UMETA(DisplayName = "Blended Aligned"),
	UnifiedSeamless UMETA(DisplayName = "Unified Seamless")
};

USTRUCT(BlueprintType)
struct FPlanetTransitionProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	EPlanetTransitionMode TransitionMode = EPlanetTransitionMode::BlendedAligned;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius")
	bool bUseAdaptiveRadii = true;

	/** If true, positive StreamingRadius/HandoffRadius values override adaptive radii. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius")
	bool bUseExplicitRadiiOverrides = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius")
	float BaseStreamingRadiusMultiplier = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius")
	float BaseHandoffRadiusMultiplier = 1.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius")
	float MinStreamingRadiusMultiplier = 1.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius")
	float MaxStreamingRadiusMultiplier = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius")
	float MinHandoffRadiusMultiplier = 1.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius")
	float MaxHandoffRadiusMultiplier = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius")
	float LoadLatencyBudgetSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius")
	float HandoffSettleBudgetSeconds = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius")
	float MinApproachLeadTimeSeconds = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius")
	float MaxAssumedApproachSpeed = 50000.f;

	/** Multiplier of planet radius used to compute adaptive exit altitude. Effective exit = max(ExitAltitude, PlanetRadius * this). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Adaptive Radius", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float ExitAltitudeMultiplier = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Blend")
	float SurfaceBlendDurationSeconds = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition|Blend")
	bool bAllowLegacyFallback = true;
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

	/**
	 * Scale applied to the streamed surface level.
	 * 0 = auto-compute from planet radius and SurfaceLevelWorldExtent.
	 * > 0 = explicit override. Reduce if the level extends beyond the planet sphere.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float SurfaceLevelScaleMultiplier = 0.f;

	/** Half-extent of surface level content in UU at scale 1.0. Used by auto-scale (SurfaceLevelScaleMultiplier == 0). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming", meta = (ClampMin = "100.0"))
	float SurfaceLevelWorldExtent = 500000.f;

	/** Fraction of planet radius the surface level patch should cover. Used by auto-scale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float DesiredPatchFraction = 0.3f;

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

	/** Per-planet transition profile. Each planet can opt into legacy or blended/unified behavior independently. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Transition")
	FPlanetTransitionProfile TransitionProfile;

	/** Planet sphere fade: multiplier of planet radius at which fade starts (e.g. 1.2 = start fading when within 1.2x radius). Fade reaches 0 at handoff. Set to 0 to disable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming|Fade")
	float FadeStartMultiplier = 0.f;

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

	/** Streaming radius currently in effect (manual override or adaptive). */
	float GetEffectiveStreamingRadius() const;

	/** True if the given squared distance is within HandoffRadius (player at surface; safe to teleport). */
	bool ShouldTransitionToSurface(float DistanceSq) const;

	/** Radial approach speed toward planet center (UU/s). */
	float GetPlayerApproachSpeedTowardPlanet() const;

	/** Adaptive size+speed streaming radius. */
	float ComputeAdaptiveStreamingRadius(float PlanetRadius, float ApproachSpeed) const;

	/** Adaptive size+speed handoff radius. */
	float ComputeAdaptiveHandoffRadius(float PlanetRadius, float ApproachSpeed) const;

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

	// --- Testable internals (public for unit tests) ---

	float GetPlanetRadiusFromOwner() const;
	float GetEffectiveHandoffRadius() const;
	float GetEffectiveExitAltitude() const;
	float ComputeAutoSurfaceLevelScale() const;
	float GetEffectiveFadeStartRadius() const;
	float GetEffectiveFullFadeRadius() const;
	float ComputeRevealProgress(float DistanceToPlayer) const;

	bool TryAcquireTransitionLock();
	void ReleaseTransitionLock();
	bool bOwnsTransitionLock = false;

	FVector SurfaceLevelWorldOrigin = FVector::ZeroVector;
	FPlanetTransitionOrientation LastTransitionOrientation;

	FVector TangentNormal = FVector::UpVector;
	FVector TangentX = FVector::ForwardVector;
	FVector TangentY = FVector::RightVector;

	void ComputeTangentFrame(const FVector& PlanetCenter);
	FVector SpaceToSurfacePosition(const FVector& SpacePos) const;
	FVector SurfaceToSpacePosition(const FVector& SurfacePos) const;
	FPlanetTransitionOrientation CaptureCurrentViewOrientation() const;
	FQuat ComputeForwardPriorityBlendedViewQuat(const FQuat& CurrentView, const FVector& TargetUp, float BlendAlpha) const;
	void ApplyAtmosphereRollBlend(float BlendAlpha);

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

	void UpdatePlanetFade(float DistanceToPlayer);
	void SetPlanetFadeAlpha(float Alpha);
	void SetPlanetRevealParams(float Progress);
	void ResetPlanetRevealParams();

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CachedPlanetFadeMaterial;

	float CurrentRevealProgress = 0.f;

	/** Seconds since we streamed out (player left surface). Used to avoid immediate re-entry flip-flop. */
	float TimeSinceStreamOut = 0.f;

	/** World time when we entered Loading state; used for load timeout. */
	float LoadingStartTime = 0.f;

	/** Minimum seconds after stream-out before we allow transition back to surface. */
	static constexpr float StreamOutReentryCooldownSeconds = 1.0f;
};
