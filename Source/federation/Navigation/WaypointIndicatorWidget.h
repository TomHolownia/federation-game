// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WaypointIndicatorWidget.generated.h"

/**
 * Lightweight data holder for a single waypoint's on-screen state.
 * The HUD uses Canvas drawing -- no UMG dependency, fully Live-Coding safe.
 */
USTRUCT(BlueprintType)
struct FEDERATION_API FWaypointIndicatorData
{
	GENERATED_BODY()

	UPROPERTY()
	FText DisplayName;

	UPROPERTY()
	FText DistanceText;

	UPROPERTY()
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	UPROPERTY()
	bool bOnScreen = false;
};
