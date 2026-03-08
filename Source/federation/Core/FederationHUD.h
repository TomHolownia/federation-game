// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FederationHUD.generated.h"

class AFederationGameState;
class UWaypointComponent;

/**
 * Developer diagnostics HUD and waypoint indicator manager.
 * Draws Speed / Level on screen and Canvas-based waypoint markers.
 */
UCLASS()
class FEDERATION_API AFederationHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFederationHUD();

	virtual void DrawHUD() override;

	// --- Dev diagnostics ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev")
	bool bShowDevDiagnostics = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev", meta = (ClampMin = "0", ClampMax = "1000"))
	float DevTextStartY = 24.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev", meta = (ClampMin = "10", ClampMax = "50"))
	float DevTextLineHeight = 22.f;

	// --- Waypoint indicators ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoints")
	bool bShowWaypoints = true;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Dev")
	void DrawDevDiagnostics(float& OutNextY);
	virtual void DrawDevDiagnostics_Implementation(float& OutNextY);

private:
	void DrawWaypointIndicators();
	void DrawWaypointMarker(const FVector2D& ScreenPos, const FText& Name, const FText& Distance);
};
