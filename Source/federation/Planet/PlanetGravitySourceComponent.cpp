// Copyright Federation Game. All Rights Reserved.

#include "Planet/PlanetGravitySourceComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

UPlanetGravitySourceComponent::UPlanetGravitySourceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UPlanetGravitySourceComponent::GetSourceRadiusUU() const
{
	const AActor* Owner = GetOwner();
	if (!Owner) return 0.f;

	const FBox Box = Owner->GetComponentsBoundingBox();
	if (Box.IsValid)
	{
		const FVector Extent = Box.GetExtent();
		return FMath::Max(Extent.X, FMath::Max(Extent.Y, Extent.Z));
	}

	if (const USceneComponent* Root = Owner->GetRootComponent())
	{
		return Root->Bounds.SphereRadius;
	}
	return 0.f;
}

float UPlanetGravitySourceComponent::ComputeGravityStrengthAtDistance(float DistanceUU) const
{
	if (!bAffectsGravity)
	{
		return 0.f;
	}

	const float Radius = FMath::Max(GetSourceRadiusUU(), 1.f);
	const float InfluenceLimit = Radius * MaxInfluenceDistanceMultiplier;
	if (MaxInfluenceDistanceMultiplier > 0.f && DistanceUU > InfluenceLimit)
	{
		return 0.f;
	}

	const float MinDistance = Radius * FMath::Max(0.1f, MinDistanceMultiplier);
	const float EffectiveDistance = FMath::Max(DistanceUU, MinDistance);
	const float Ratio = Radius / EffectiveDistance;
	return FMath::Max(0.f, SurfaceGravityScale) * FMath::Pow(Ratio, FMath::Max(0.1f, FalloffExponent));
}
