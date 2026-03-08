// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlanetSurfaceStressTest.generated.h"

class UInstancedStaticMeshComponent;

/**
 * Spawns configurable numbers of instanced static meshes across a radius
 * to stress-test World Partition streaming and rendering performance.
 *
 * Place in a planet surface level. Adjust InstanceCount at runtime via
 * the console command "Fed.StressTest.SetCount <N>" or in the Details panel.
 */
UCLASS(ClassGroup = "Federation")
class FEDERATION_API APlanetSurfaceStressTest : public AActor
{
	GENERATED_BODY()

public:
	APlanetSurfaceStressTest();

	/** Number of mesh instances to scatter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest", meta = (ClampMin = "0"))
	int32 InstanceCount = 10000;

	/** Radius around this actor to scatter instances. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest", meta = (ClampMin = "1.0"))
	float ScatterRadius = 400000.f;

	/** Mesh to instance. If null, uses the engine default sphere. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	TObjectPtr<UStaticMesh> InstanceMesh;

	/** Scale range for scattered instances (uniform, random between min and max). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	FVector2D ScaleRange = FVector2D(0.5f, 2.f);

	/** Seed for deterministic scattering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	int32 RandomSeed = 12345;

	/** Instanced mesh component holding all scattered instances. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StressTest")
	TObjectPtr<UInstancedStaticMeshComponent> InstancedMeshComp;

	/** Clear and re-scatter all instances using current settings. */
	UFUNCTION(BlueprintCallable, Category = "StressTest")
	void RegenerateInstances();

	/** Returns the current actual instance count (may differ from InstanceCount during regeneration). */
	UFUNCTION(BlueprintCallable, Category = "StressTest")
	int32 GetActualInstanceCount() const;

protected:
	virtual void BeginPlay() override;

private:
	void ScatterInstances();
	void LogPerformanceStats() const;
};
