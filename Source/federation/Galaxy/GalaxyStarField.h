// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GalaxyStarField.generated.h"

/**
 * AGalaxyStarField - Renders a galaxy star field using instanced static meshes.
 * 
 * This actor generates and displays thousands of stars in a spiral galaxy pattern
 * using UInstancedStaticMeshComponent for optimal performance.
 * 
 * Features:
 * - Supports 10,000+ stars at 60fps
 * - Procedural spiral galaxy generation
 * - Per-instance color variation via custom data
 * - Configurable galaxy parameters
 */
UCLASS()
class FEDERATION_API AGalaxyStarField : public AActor
{
	GENERATED_BODY()
	
public:	
	AGalaxyStarField();

	/** Regenerates the star field with current parameters */
	UFUNCTION(BlueprintCallable, Category = "Galaxy")
	void RegenerateStars();

	/** Returns the current number of star instances */
	UFUNCTION(BlueprintPure, Category = "Galaxy")
	int32 GetStarCount() const;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	/** Generates star positions in a spiral galaxy pattern */
	void GenerateSpiralGalaxy();
	
	/** Calculates a position on a spiral arm */
	FVector CalculateSpiralArmPosition(float ArmAngle, float Distance, float RandomOffset) const;
	
	/** Gets a color based on star temperature (blue-white-yellow-orange-red) */
	FLinearColor GetStarColor(float Temperature) const;

public:
	// --- Galaxy Shape Parameters ---
	
	/** Total number of stars to generate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Shape", meta = (ClampMin = "100", ClampMax = "100000"))
	int32 StarCount = 10000;
	
	/** Radius of the galaxy in world units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Shape", meta = (ClampMin = "1000.0"))
	float GalaxyRadius = 50000.0f;
	
	/** Thickness of the galaxy disk */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Shape", meta = (ClampMin = "100.0"))
	float GalaxyThickness = 2000.0f;
	
	/** Number of spiral arms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Shape", meta = (ClampMin = "1", ClampMax = "8"))
	int32 SpiralArmCount = 4;
	
	/** How tightly wound the spiral arms are */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Shape", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float SpiralTightness = 0.5f;
	
	/** Amount of random spread for stars (0 = on arm, 1 = very spread) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Shape", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ArmSpread = 0.3f;
	
	/** Percentage of stars in the galactic core vs arms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Shape", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CoreDensity = 0.3f;

	// --- Visual Parameters ---
	
	/** Random seed for reproducible generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Visual")
	int32 RandomSeed = 12345;
	
	/** Scale of individual star meshes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Visual", meta = (ClampMin = "0.1"))
	float StarScale = 50.0f;
	
	/** Minimum star scale multiplier for variation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Visual", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MinStarScaleMultiplier = 0.5f;
	
	/** Maximum star scale multiplier for variation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Visual", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float MaxStarScaleMultiplier = 2.0f;

	// --- Components ---
	
	/** The instanced mesh component that renders all stars */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInstancedStaticMeshComponent> StarMeshComponent;
	
	/** Static mesh to use for stars (should be a simple sphere or billboard) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Visual")
	TObjectPtr<UStaticMesh> StarMesh;
	
	/** Material to use for stars (should read custom data for color) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Galaxy|Visual")
	TObjectPtr<UMaterialInterface> StarMaterial;
};
