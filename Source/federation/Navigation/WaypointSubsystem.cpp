// Copyright Federation Game. All Rights Reserved.

#include "Navigation/WaypointSubsystem.h"
#include "Navigation/WaypointComponent.h"

void UWaypointSubsystem::RegisterWaypoint(UWaypointComponent* Waypoint)
{
	if (!Waypoint) return;

	for (const TWeakObjectPtr<UWaypointComponent>& Existing : RegisteredWaypoints)
	{
		if (Existing.Get() == Waypoint) return;
	}
	RegisteredWaypoints.Add(Waypoint);
}

void UWaypointSubsystem::UnregisterWaypoint(UWaypointComponent* Waypoint)
{
	RegisteredWaypoints.RemoveAll([Waypoint](const TWeakObjectPtr<UWaypointComponent>& Entry)
	{
		return !Entry.IsValid() || Entry.Get() == Waypoint;
	});
}

TArray<UWaypointComponent*> UWaypointSubsystem::GetAllActiveWaypoints() const
{
	TArray<UWaypointComponent*> Result;
	for (const TWeakObjectPtr<UWaypointComponent>& Weak : RegisteredWaypoints)
	{
		UWaypointComponent* Comp = Weak.Get();
		if (Comp && Comp->IsWaypointEnabled())
		{
			Result.Add(Comp);
		}
	}
	return Result;
}

TArray<UWaypointComponent*> UWaypointSubsystem::GetWaypointsByType(EWaypointType Type) const
{
	TArray<UWaypointComponent*> Result;
	for (const TWeakObjectPtr<UWaypointComponent>& Weak : RegisteredWaypoints)
	{
		UWaypointComponent* Comp = Weak.Get();
		if (Comp && Comp->IsWaypointEnabled() && Comp->GetWaypointType() == Type)
		{
			Result.Add(Comp);
		}
	}
	return Result;
}

UWaypointComponent* UWaypointSubsystem::GetNearestWaypoint(FVector Location) const
{
	UWaypointComponent* Nearest = nullptr;
	double BestDistSq = TNumericLimits<double>::Max();

	for (const TWeakObjectPtr<UWaypointComponent>& Weak : RegisteredWaypoints)
	{
		UWaypointComponent* Comp = Weak.Get();
		if (!Comp || !Comp->IsWaypointEnabled()) continue;

		AActor* Owner = Comp->GetOwner();
		if (!Owner) continue;

		const double DistSq = FVector::DistSquared(Owner->GetActorLocation(), Location);
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = Comp;
		}
	}
	return Nearest;
}
