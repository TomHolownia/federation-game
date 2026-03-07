// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WaypointTypes.generated.h"

/** Classification for waypoints, used for filtering and icon selection. */
UENUM(BlueprintType)
enum class EWaypointType : uint8
{
	Planet    UMETA(DisplayName = "Planet"),
	Station   UMETA(DisplayName = "Station"),
	Ship      UMETA(DisplayName = "Ship"),
	Objective UMETA(DisplayName = "Objective"),
	Custom    UMETA(DisplayName = "Custom"),
};

namespace WaypointConstants
{
	/** UE units per kilometre (1 UU = 1 cm). */
	constexpr double UUPerKm = 100000.0;

	/** UE units per metre. */
	constexpr double UUPerMetre = 100.0;

	/** Distance threshold (in UU) below which we display metres instead of km. */
	constexpr double MetreThresholdUU = UUPerKm; // 1 km
}
