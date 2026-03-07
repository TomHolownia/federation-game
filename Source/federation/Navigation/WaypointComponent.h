// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Navigation/WaypointTypes.h"
#include "WaypointComponent.generated.h"

class UTexture2D;

/**
 * Attach to any actor to make it a navigation waypoint.
 * Automatically registers with UWaypointSubsystem on BeginPlay.
 */
UCLASS(ClassGroup = "Federation", meta = (BlueprintSpawnableComponent, DisplayName = "Waypoint"))
class FEDERATION_API UWaypointComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWaypointComponent();

	/** Name shown on the HUD indicator. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FText DisplayName;

	/** Optional icon texture. If null the HUD uses a default marker. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	TObjectPtr<UTexture2D> WaypointIcon;

	/** Classification for filtering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	EWaypointType WaypointType = EWaypointType::Custom;

	/** When false the waypoint is hidden without removing the component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	bool bWaypointEnabled = true;

	/** Max distance (UU) at which the indicator is visible. 0 = always visible. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint", meta = (ClampMin = "0"))
	double MaxVisibleDistance = 0.0;

	UFUNCTION(BlueprintPure, Category = "Waypoint")
	bool IsWaypointEnabled() const { return bWaypointEnabled; }

	UFUNCTION(BlueprintPure, Category = "Waypoint")
	EWaypointType GetWaypointType() const { return WaypointType; }

	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	void SetWaypointEnabled(bool bEnabled);

	/** World location of the waypoint (owner's location). */
	UFUNCTION(BlueprintPure, Category = "Waypoint")
	FVector GetWaypointLocation() const;

	/** Format a UU distance as human-readable km / m text. */
	UFUNCTION(BlueprintPure, Category = "Waypoint")
	static FText FormatDistance(double DistanceUU);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
