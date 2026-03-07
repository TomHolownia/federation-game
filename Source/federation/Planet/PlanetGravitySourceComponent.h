// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlanetGravitySourceComponent.generated.h"

/**
 * Gravity source definition for celestial bodies.
 * Attach to planets/moons and tune falloff without changing consumer logic.
 */
UCLASS(ClassGroup = "Federation", meta = (BlueprintSpawnableComponent))
class FEDERATION_API UPlanetGravitySourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlanetGravitySourceComponent();

	/** Enables/disables this source without removing the component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Source")
	bool bAffectsGravity = true;

	/** Relative gravity strength at the body's surface (1.0 ~= default world gravity scale). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Source", meta = (ClampMin = "0.0"))
	float SurfaceGravityScale = 1.0f;

	/** Inverse-distance falloff exponent. 2.0 approximates inverse-square behavior. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Source", meta = (ClampMin = "0.1"))
	float FalloffExponent = 2.0f;

	/** Clamp near-surface sampling distance to Radius * multiplier to avoid singularity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Source", meta = (ClampMin = "0.1"))
	float MinDistanceMultiplier = 1.0f;

	/** Optional hard cutoff. 0 disables cutoff (infinite influence). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Source", meta = (ClampMin = "0.0"))
	float MaxInfluenceDistanceMultiplier = 0.0f;

	/** If > 0, overrides the bounding-box-derived radius (useful for actors without visible geometry). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Source", meta = (ClampMin = "0.0"))
	float ManualRadius = 0.f;

	UFUNCTION(BlueprintCallable, Category = "Gravity Source")
	float GetSourceRadiusUU() const;

	UFUNCTION(BlueprintCallable, Category = "Gravity Source")
	float ComputeGravityStrengthAtDistance(float DistanceUU) const;
};
