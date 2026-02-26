// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FederationHUD.generated.h"

class AFederationGameState;

/**
 * Developer diagnostics HUD. Draws Speed and Level (streaming state) on screen.
 * Extend this class or add more Draw* methods for additional diagnostics.
 */
UCLASS()
class FEDERATION_API AFederationHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFederationHUD();

	virtual void DrawHUD() override;

	/** When true, draw developer diagnostics (Speed, Level). Toggle at runtime if needed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev")
	bool bShowDevDiagnostics = true;

	/** Vertical start position for the first line of dev text (from top of screen). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev", meta = (ClampMin = "0", ClampMax = "1000"))
	float DevTextStartY = 24.f;

	/** Line height for dev text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev", meta = (ClampMin = "10", ClampMax = "50"))
	float DevTextLineHeight = 22.f;

protected:
	/** Draw a single dev line (Speed, Level). Override or call from DrawHUD to add more lines. */
	UFUNCTION(BlueprintNativeEvent, Category = "Dev")
	void DrawDevDiagnostics(float& OutNextY);
	virtual void DrawDevDiagnostics_Implementation(float& OutNextY);
};