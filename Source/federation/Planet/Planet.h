// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Planet.generated.h"

class UPlanetGravitySourceComponent;
class UWaypointComponent;

/**
 * Planet actor representing a celestial body in the solar system level.
 * All planets exist as sphere meshes in the single shared world space — there
 * are no separate surface levels. Terrain and surface detail stream in via the
 * spherical quadtree LOD system as the player approaches.
 *
 * Assign your sphere mesh to the static mesh component. Gravity falloff and
 * sphere-of-influence are configured on PlanetGravitySource.
 */
UCLASS(ClassGroup = "Federation", meta = (DisplayName = "Planet"))
class FEDERATION_API APlanet : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	APlanet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Per-planet gravity source tuning. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Planet")
	TObjectPtr<UPlanetGravitySourceComponent> PlanetGravitySource;

	/** Display name shown on the waypoint indicator HUD. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	FText PlanetName;

	/** Navigation waypoint -- auto-created; displays PlanetName on the HUD. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Planet")
	TObjectPtr<UWaypointComponent> WaypointComp;

	virtual void PostInitializeComponents() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
