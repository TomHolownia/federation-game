// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SkySphere.generated.h"

class UStaticMeshComponent;

/**
 * ASkySphere - A large sphere used as a skybox background (e.g. Milky Way).
 *
 * Place at origin with a large scale so the camera is inside the sphere.
 * Assign an unlit, two-sided material (e.g. equirectangular Milky Way texture).
 * Can be placed via Config/PlacementData.json and Tools -> Place Actors From Data.
 *
 * Free high-res Milky Way sources: NASA SVS GLIMPSE 360, NOIRLab Maunakea (CC0).
 * See docs/technical/ai-workflow-and-galaxy-scale.md (Skybox section).
 */
UCLASS(Blueprintable, Placeable, Category = "Skybox")
class FEDERATION_API ASkySphere : public AActor
{
	GENERATED_BODY()

public:
	ASkySphere();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	/** Static mesh component (sphere). Scale the actor large so the camera is inside. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SphereMesh;

	/**
	 * Material for the sky (unlit, two-sided).
	 * Use an equirectangular Milky Way texture for a "in space" look.
	 * If null, a fallback dark material is used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skybox")
	TObjectPtr<UMaterialInterface> SkyMaterial;

	/** Applies SkyMaterial to the sphere (call after setting SkyMaterial from code/data). */
	UFUNCTION(BlueprintCallable, Category = "Skybox")
	void UpdateSkyMaterial();
};
