// Copyright Federation Game. All Rights Reserved.

#include "Planet/Planet.h"
#include "Planet/PlanetGravitySourceComponent.h"
#include "Planet/PlanetSurfaceStreamer.h"
#include "Navigation/WaypointComponent.h"
#include "Navigation/WaypointTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

APlanet::APlanet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Tags.AddUnique(FName(TEXT("Planet")));
	PlanetSurfaceStreamer = CreateDefaultSubobject<UPlanetSurfaceStreamer>(TEXT("PlanetSurfaceStreamer"));
	PlanetGravitySource = CreateDefaultSubobject<UPlanetGravitySourceComponent>(TEXT("PlanetGravitySource"));

	WaypointComp = CreateDefaultSubobject<UWaypointComponent>(TEXT("Waypoint"));
	WaypointComp->WaypointType = EWaypointType::Planet;
	WaypointComp->DisplayName = FText::FromString(TEXT("Unknown Planet"));

	// Use adaptive streaming/handoff radius by default (0 = use planet size + approach speed).
	PlanetSurfaceStreamer->StreamingRadius = 0.f;

	// Ensure APlanet has a sphere visual even when placement data omits/invalidates mesh path.
	if (UStaticMeshComponent* SMComp = GetStaticMeshComponent())
	{
		if (!SMComp->GetStaticMesh())
		{
			UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
			if (!SphereMesh)
			{
				SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Shape_Sphere.Shape_Sphere"));
			}
			if (SphereMesh)
			{
				SMComp->SetStaticMesh(SphereMesh);
			}
		}
	}
}

void APlanet::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (WaypointComp && !PlanetName.IsEmpty())
	{
		WaypointComp->DisplayName = PlanetName;
	}
}

#if WITH_EDITOR
void APlanet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropName = PropertyChangedEvent.GetPropertyName();
	if (PropName == GET_MEMBER_NAME_CHECKED(APlanet, PlanetName) && WaypointComp)
	{
		WaypointComp->DisplayName = PlanetName;
	}
}
#endif
