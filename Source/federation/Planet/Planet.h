// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Planet.generated.h"

class UPlanetSurfaceStreamer;

/**
 * Planet actor that always has a PlanetSurfaceStreamer attached.
 * Use this instead of a plain StaticMeshActor for any celestial body that
 * supports surface streaming. The streamer is created by default so you don't
 * need to add it manually after each editor restart.
 *
 * Assign your planet mesh to the static mesh component as usual. Set
 * SurfaceLevelPath on the PlanetSurfaceStreamer component to the level to
 * stream when the player approaches. StreamingRadius = 0 (default) uses
 * adaptive radius from planet size and approach speed.
 */
UCLASS(ClassGroup = "Federation", meta = (DisplayName = "Planet"))
class FEDERATION_API APlanet : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	APlanet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Streamer is always present; configure SurfaceLevelPath and options in Details. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Planet")
	TObjectPtr<UPlanetSurfaceStreamer> PlanetSurfaceStreamer;
};
