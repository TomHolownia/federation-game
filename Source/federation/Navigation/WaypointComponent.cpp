// Copyright Federation Game. All Rights Reserved.

#include "Navigation/WaypointComponent.h"
#include "Navigation/WaypointSubsystem.h"
#include "GameFramework/Actor.h"

UWaypointComponent::UWaypointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWaypointComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UWaypointSubsystem* Sub = World->GetSubsystem<UWaypointSubsystem>())
		{
			Sub->RegisterWaypoint(this);
		}
	}
}

void UWaypointComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UWaypointSubsystem* Sub = World->GetSubsystem<UWaypointSubsystem>())
		{
			Sub->UnregisterWaypoint(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UWaypointComponent::SetWaypointEnabled(bool bEnabled)
{
	bWaypointEnabled = bEnabled;
}

FVector UWaypointComponent::GetWaypointLocation() const
{
	AActor* Owner = GetOwner();
	return Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
}

FText UWaypointComponent::FormatDistance(double DistanceUU)
{
	if (DistanceUU >= WaypointConstants::MetreThresholdUU)
	{
		const double Km = DistanceUU / WaypointConstants::UUPerKm;
		if (Km >= 100.0)
		{
			return FText::FromString(FString::Printf(TEXT("%.0f km"), Km));
		}
		if (Km >= 10.0)
		{
			return FText::FromString(FString::Printf(TEXT("%.1f km"), Km));
		}
		return FText::FromString(FString::Printf(TEXT("%.2f km"), Km));
	}

	const double Metres = DistanceUU / WaypointConstants::UUPerMetre;
	return FText::FromString(FString::Printf(TEXT("%.0f m"), Metres));
}
