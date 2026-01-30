// Copyright Federation Game. All Rights Reserved.

#include "GalaxyStarField.h"
#include "Components/InstancedStaticMeshComponent.h"

AGalaxyStarField::AGalaxyStarField()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create the instanced mesh component
	StarMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("StarMeshComponent"));
	RootComponent = StarMeshComponent;
	
	// Configure for optimal performance with many instances
	StarMeshComponent->SetMobility(EComponentMobility::Static);
	StarMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StarMeshComponent->SetCastShadow(false);
	
	// Set up custom data for per-instance color variation
	// We use 4 floats: R, G, B, Intensity
	StarMeshComponent->NumCustomDataFloats = 4;
}

void AGalaxyStarField::BeginPlay()
{
	Super::BeginPlay();
}

void AGalaxyStarField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	// Regenerate stars when properties change in editor
	RegenerateStars();
}

void AGalaxyStarField::RegenerateStars()
{
	if (!StarMeshComponent)
	{
		return;
	}
	
	// Apply mesh and material if set
	if (StarMesh)
	{
		StarMeshComponent->SetStaticMesh(StarMesh);
	}
	
	if (StarMaterial)
	{
		StarMeshComponent->SetMaterial(0, StarMaterial);
	}
	
	// Clear existing instances
	StarMeshComponent->ClearInstances();
	
	// Generate the galaxy
	GenerateSpiralGalaxy();
}

int32 AGalaxyStarField::GetStarCount() const
{
	if (StarMeshComponent)
	{
		return StarMeshComponent->GetInstanceCount();
	}
	return 0;
}

void AGalaxyStarField::GenerateSpiralGalaxy()
{
	if (!StarMeshComponent || StarCount <= 0)
	{
		return;
	}
	
	// Initialize random stream with seed for reproducibility
	FRandomStream RandomStream(RandomSeed);
	
	// Pre-allocate instance transforms for better performance
	TArray<FTransform> InstanceTransforms;
	InstanceTransforms.Reserve(StarCount);
	
	// Store color data to apply after adding instances
	TArray<FLinearColor> StarColors;
	StarColors.Reserve(StarCount);
	
	// Calculate how many stars go in the core vs arms
	const int32 CoreStarCount = FMath::RoundToInt(StarCount * CoreDensity);
	const int32 ArmStarCount = StarCount - CoreStarCount;
	const int32 StarsPerArm = SpiralArmCount > 0 ? ArmStarCount / SpiralArmCount : 0;
	
	// Generate core stars (dense central region)
	for (int32 i = 0; i < CoreStarCount; ++i)
	{
		// Core stars are distributed in a sphere with higher density toward center
		const float CoreRadius = GalaxyRadius * 0.2f; // Core is 20% of galaxy radius
		const float Distance = FMath::Pow(RandomStream.FRand(), 2.0f) * CoreRadius;
		const float Theta = RandomStream.FRand() * 2.0f * PI;
		const float Phi = FMath::Acos(2.0f * RandomStream.FRand() - 1.0f);
		
		FVector Position;
		Position.X = Distance * FMath::Sin(Phi) * FMath::Cos(Theta);
		Position.Y = Distance * FMath::Sin(Phi) * FMath::Sin(Theta);
		Position.Z = Distance * FMath::Cos(Phi) * (GalaxyThickness / GalaxyRadius); // Flatten to disk
		
		// Random scale variation
		const float ScaleMultiplier = RandomStream.FRandRange(MinStarScaleMultiplier, MaxStarScaleMultiplier);
		const float FinalScale = StarScale * ScaleMultiplier;
		
		FTransform StarTransform;
		StarTransform.SetLocation(Position);
		StarTransform.SetScale3D(FVector(FinalScale));
		InstanceTransforms.Add(StarTransform);
		
		// Core stars tend to be older (redder/yellower)
		const float Temperature = RandomStream.FRandRange(0.2f, 0.6f);
		StarColors.Add(GetStarColor(Temperature));
	}
	
	// Generate spiral arm stars
	const float ArmAngleStep = 2.0f * PI / FMath::Max(1, SpiralArmCount);
	
	for (int32 ArmIndex = 0; ArmIndex < SpiralArmCount; ++ArmIndex)
	{
		const float ArmBaseAngle = ArmIndex * ArmAngleStep;
		
		for (int32 StarIndex = 0; StarIndex < StarsPerArm; ++StarIndex)
		{
			// Distance from center (weighted toward outer regions for better arm visibility)
			const float DistanceRatio = 0.2f + RandomStream.FRand() * 0.8f; // Start at 20% radius
			const float Distance = DistanceRatio * GalaxyRadius;
			
			// Calculate position on spiral arm with spread
			const float SpreadAmount = ArmSpread * Distance * RandomStream.FRandRange(-1.0f, 1.0f);
			FVector Position = CalculateSpiralArmPosition(ArmBaseAngle, Distance, SpreadAmount);
			
			// Add vertical spread (thinner toward edges)
			const float HeightSpread = GalaxyThickness * (1.0f - DistanceRatio * 0.5f);
			Position.Z = RandomStream.FRandRange(-HeightSpread, HeightSpread) * 0.5f;
			
			// Random scale variation
			const float ScaleMultiplier = RandomStream.FRandRange(MinStarScaleMultiplier, MaxStarScaleMultiplier);
			const float FinalScale = StarScale * ScaleMultiplier;
			
			FTransform StarTransform;
			StarTransform.SetLocation(Position);
			StarTransform.SetScale3D(FVector(FinalScale));
			InstanceTransforms.Add(StarTransform);
			
			// Arm stars have wider temperature range (more young blue stars)
			const float Temperature = RandomStream.FRandRange(0.0f, 1.0f);
			StarColors.Add(GetStarColor(Temperature));
		}
	}
	
	// Add remaining stars (from integer division) as scattered field stars
	const int32 RemainingStars = StarCount - InstanceTransforms.Num();
	for (int32 i = 0; i < RemainingStars; ++i)
	{
		const float Distance = FMath::Sqrt(RandomStream.FRand()) * GalaxyRadius;
		const float Angle = RandomStream.FRand() * 2.0f * PI;
		
		FVector Position;
		Position.X = Distance * FMath::Cos(Angle);
		Position.Y = Distance * FMath::Sin(Angle);
		Position.Z = RandomStream.FRandRange(-GalaxyThickness, GalaxyThickness) * 0.3f;
		
		const float ScaleMultiplier = RandomStream.FRandRange(MinStarScaleMultiplier, MaxStarScaleMultiplier);
		const float FinalScale = StarScale * ScaleMultiplier;
		
		FTransform StarTransform;
		StarTransform.SetLocation(Position);
		StarTransform.SetScale3D(FVector(FinalScale));
		InstanceTransforms.Add(StarTransform);
		
		const float Temperature = RandomStream.FRandRange(0.0f, 1.0f);
		StarColors.Add(GetStarColor(Temperature));
	}
	
	// Batch add all instances for better performance
	StarMeshComponent->AddInstances(InstanceTransforms, false);
	
	// Apply per-instance custom data for color
	for (int32 i = 0; i < StarColors.Num(); ++i)
	{
		const FLinearColor& Color = StarColors[i];
		StarMeshComponent->SetCustomDataValue(i, 0, Color.R);
		StarMeshComponent->SetCustomDataValue(i, 1, Color.G);
		StarMeshComponent->SetCustomDataValue(i, 2, Color.B);
		StarMeshComponent->SetCustomDataValue(i, 3, 1.0f); // Intensity
	}
}

FVector AGalaxyStarField::CalculateSpiralArmPosition(float ArmAngle, float Distance, float RandomOffset) const
{
	// Logarithmic spiral: r = a * e^(b*theta)
	// We invert this to get theta from r: theta = ln(r/a) / b
	// But for simplicity, we use: theta = ArmAngle + SpiralTightness * Distance / GalaxyRadius * 2*PI
	
	const float SpiralAngle = ArmAngle + SpiralTightness * (Distance / GalaxyRadius) * 2.0f * PI;
	
	FVector Position;
	Position.X = (Distance + RandomOffset) * FMath::Cos(SpiralAngle);
	Position.Y = (Distance + RandomOffset) * FMath::Sin(SpiralAngle);
	Position.Z = 0.0f; // Will be set by caller
	
	return Position;
}

FLinearColor AGalaxyStarField::GetStarColor(float Temperature) const
{
	// Temperature: 0 = hot blue, 0.5 = white, 1 = cool red
	// Based on stellar classification: O-B-A-F-G-K-M
	
	FLinearColor Color;
	
	if (Temperature < 0.2f)
	{
		// O/B class: Blue-white (hot)
		const float T = Temperature / 0.2f;
		Color = FLinearColor::LerpUsingHSV(
			FLinearColor(0.6f, 0.8f, 1.0f), // Blue-white
			FLinearColor(0.8f, 0.9f, 1.0f), // White-blue
			T
		);
	}
	else if (Temperature < 0.4f)
	{
		// A/F class: White to yellow-white
		const float T = (Temperature - 0.2f) / 0.2f;
		Color = FLinearColor::LerpUsingHSV(
			FLinearColor(0.8f, 0.9f, 1.0f), // White-blue
			FLinearColor(1.0f, 1.0f, 0.9f), // White-yellow
			T
		);
	}
	else if (Temperature < 0.6f)
	{
		// G class: Yellow (like our Sun)
		const float T = (Temperature - 0.4f) / 0.2f;
		Color = FLinearColor::LerpUsingHSV(
			FLinearColor(1.0f, 1.0f, 0.9f), // White-yellow
			FLinearColor(1.0f, 0.9f, 0.6f), // Yellow
			T
		);
	}
	else if (Temperature < 0.8f)
	{
		// K class: Orange
		const float T = (Temperature - 0.6f) / 0.2f;
		Color = FLinearColor::LerpUsingHSV(
			FLinearColor(1.0f, 0.9f, 0.6f), // Yellow
			FLinearColor(1.0f, 0.7f, 0.4f), // Orange
			T
		);
	}
	else
	{
		// M class: Red (cool)
		const float T = (Temperature - 0.8f) / 0.2f;
		Color = FLinearColor::LerpUsingHSV(
			FLinearColor(1.0f, 0.7f, 0.4f), // Orange
			FLinearColor(1.0f, 0.4f, 0.3f), // Red
			T
		);
	}
	
	return Color;
}
