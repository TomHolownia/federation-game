// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Navigation/WaypointTypes.h"
#include "WaypointSubsystem.generated.h"

class UWaypointComponent;

/**
 * Passive registry for all waypoints in the world.
 * Auto-created per UWorld; no manual wiring needed.
 * WaypointComponents register/unregister themselves.
 */
UCLASS()
class FEDERATION_API UWaypointSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterWaypoint(UWaypointComponent* Waypoint);
	void UnregisterWaypoint(UWaypointComponent* Waypoint);

	/** All waypoints whose bIsActive is true (stale weak pointers pruned automatically). */
	UFUNCTION(BlueprintCallable, Category = "Waypoints")
	TArray<UWaypointComponent*> GetAllActiveWaypoints() const;

	/** Active waypoints filtered by type. */
	UFUNCTION(BlueprintCallable, Category = "Waypoints")
	TArray<UWaypointComponent*> GetWaypointsByType(EWaypointType Type) const;

	/** Nearest active waypoint to Location, or nullptr. */
	UFUNCTION(BlueprintCallable, Category = "Waypoints")
	UWaypointComponent* GetNearestWaypoint(FVector Location) const;

private:
	TArray<TWeakObjectPtr<UWaypointComponent>> RegisteredWaypoints;
};
