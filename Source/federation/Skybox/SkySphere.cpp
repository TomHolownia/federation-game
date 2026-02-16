// Copyright Federation Game. All Rights Reserved.

#include "Skybox/SkySphere.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ASkySphere::ASkySphere()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	RootComponent = SphereMesh;

	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Sphere)
	{
		SphereMesh->SetStaticMesh(Sphere);
	}

	SphereMesh->SetMobility(EComponentMobility::Static);
	SphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereMesh->SetCastShadow(false);

	// Default scale: large so the camera is inside the sphere (skybox)
	SetActorScale3D(FVector(5000.0));
}

void ASkySphere::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!SphereMesh) return;

	if (SkyMaterial)
	{
		SphereMesh->SetMaterial(0, SkyMaterial);
	}
	else
	{
		// Fallback: dark material so we do not show default grey
		UMaterialInterface* Fallback = LoadObject<UMaterialInterface>(nullptr,
			TEXT("/Game/Federation/Materials/M_GalaxyStar.M_GalaxyStar"));
		if (!Fallback)
		{
			Fallback = LoadObject<UMaterialInterface>(nullptr,
				TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
		}
		SphereMesh->SetMaterial(0, Fallback);
	}
}

void ASkySphere::UpdateSkyMaterial()
{
	if (!SphereMesh) return;
	if (SkyMaterial)
	{
		SphereMesh->SetMaterial(0, SkyMaterial);
	}
	else
	{
		UMaterialInterface* Fallback = LoadObject<UMaterialInterface>(nullptr,
			TEXT("/Game/Federation/Materials/M_GalaxyStar.M_GalaxyStar"));
		if (!Fallback)
		{
			Fallback = LoadObject<UMaterialInterface>(nullptr,
				TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
		}
		SphereMesh->SetMaterial(0, Fallback);
	}
}
