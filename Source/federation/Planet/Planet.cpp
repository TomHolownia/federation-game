// Copyright Federation Game. All Rights Reserved.

#include "Planet/Planet.h"
#include "Planet/PlanetSurfaceStreamer.h"

APlanet::APlanet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlanetSurfaceStreamer = CreateDefaultSubobject<UPlanetSurfaceStreamer>(TEXT("PlanetSurfaceStreamer"));
	// Use adaptive streaming/handoff radius by default (0 = use planet size + approach speed).
	PlanetSurfaceStreamer->StreamingRadius = 0.f;
}
