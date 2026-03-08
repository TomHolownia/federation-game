// Copyright Federation Game. All Rights Reserved.

#include "Planet/PlanetSurfaceStressTest.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "HAL/IConsoleManager.h"

static APlanetSurfaceStressTest* GStressTestInstance = nullptr;

static FAutoConsoleCommand CmdSetCount(
	TEXT("Fed.StressTest.SetCount"),
	TEXT("Set instance count for the planet surface stress test. Usage: Fed.StressTest.SetCount <N>"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		if (Args.Num() < 1 || !GStressTestInstance)
		{
			UE_LOG(LogTemp, Warning, TEXT("Usage: Fed.StressTest.SetCount <N> (requires a PlanetSurfaceStressTest actor in the level)"));
			return;
		}
		const int32 NewCount = FCString::Atoi(*Args[0]);
		GStressTestInstance->InstanceCount = FMath::Max(0, NewCount);
		GStressTestInstance->RegenerateInstances();
	})
);

APlanetSurfaceStressTest::APlanetSurfaceStressTest()
{
	PrimaryActorTick.bCanEverTick = false;

	InstancedMeshComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
	RootComponent = InstancedMeshComp;
	InstancedMeshComp->SetMobility(EComponentMobility::Static);
}

void APlanetSurfaceStressTest::BeginPlay()
{
	Super::BeginPlay();
	GStressTestInstance = this;

	if (!InstanceMesh)
	{
		InstanceMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Shape_Sphere.Shape_Sphere"));
	}

	if (InstanceMesh && InstancedMeshComp)
	{
		InstancedMeshComp->SetStaticMesh(InstanceMesh);
	}

	ScatterInstances();
	LogPerformanceStats();
}

void APlanetSurfaceStressTest::RegenerateInstances()
{
	if (InstancedMeshComp)
	{
		InstancedMeshComp->ClearInstances();
	}
	ScatterInstances();
	LogPerformanceStats();
}

int32 APlanetSurfaceStressTest::GetActualInstanceCount() const
{
	return InstancedMeshComp ? InstancedMeshComp->GetInstanceCount() : 0;
}

void APlanetSurfaceStressTest::ScatterInstances()
{
	if (!InstancedMeshComp || InstanceCount <= 0) return;

	FRandomStream Rng(RandomSeed);
	const FVector Origin = GetActorLocation();

	InstancedMeshComp->ClearInstances();

	for (int32 i = 0; i < InstanceCount; ++i)
	{
		const float Angle = Rng.FRandRange(0.f, 2.f * PI);
		const float Dist = Rng.FRandRange(0.f, ScatterRadius);
		const float X = Origin.X + Dist * FMath::Cos(Angle);
		const float Y = Origin.Y + Dist * FMath::Sin(Angle);
		const float Z = Origin.Z;

		const float Scale = Rng.FRandRange(ScaleRange.X, ScaleRange.Y);
		const float Yaw = Rng.FRandRange(0.f, 360.f);

		FTransform InstanceTransform;
		InstanceTransform.SetLocation(FVector(X, Y, Z) - Origin);
		InstanceTransform.SetRotation(FQuat(FVector::UpVector, FMath::DegreesToRadians(Yaw)));
		InstanceTransform.SetScale3D(FVector(Scale));

		InstancedMeshComp->AddInstance(InstanceTransform, /*bWorldSpace=*/false);
	}
}

void APlanetSurfaceStressTest::LogPerformanceStats() const
{
	const int32 ActualCount = GetActualInstanceCount();
	const FBox Bounds = InstancedMeshComp ? InstancedMeshComp->Bounds.GetBox() : FBox(ForceInit);
	const FVector BoundsSize = Bounds.GetSize();

	UE_LOG(LogTemp, Log,
		TEXT("PlanetSurfaceStressTest: %d instances | ScatterRadius=%.0f | BoundsSize=(%.0f, %.0f, %.0f) | Seed=%d"),
		ActualCount, ScatterRadius, BoundsSize.X, BoundsSize.Y, BoundsSize.Z, RandomSeed);
}
