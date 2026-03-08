// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FederationHUD.generated.h"

class AFederationGameState;
class UWaypointComponent;
class UDevDiagnosticsWidget;
class UInventoryWidget;

/**
 * Main HUD manager.
 * Owns UMG widgets for dev diagnostics and inventory.
 * Waypoint indicators are still drawn via Canvas for efficiency.
 */
UCLASS()
class FEDERATION_API AFederationHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFederationHUD();

	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	// --- Widget toggles ---

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ToggleDevDiagnostics();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HUD")
	bool IsInventoryOpen() const;

	// --- Waypoint indicators (Canvas) ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoints")
	bool bShowWaypoints = true;

private:
	void DrawWaypointIndicators();
	void DrawWaypointMarker(const FVector2D& ScreenPos, const FText& Name, const FText& Distance);

	UPROPERTY()
	TObjectPtr<UDevDiagnosticsWidget> DevDiagnosticsWidget;

	UPROPERTY()
	TObjectPtr<UInventoryWidget> InventoryWidget;
};
