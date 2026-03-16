// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DevDiagnosticsWidget.generated.h"

class UTextBlock;

/**
 * Developer-only diagnostics overlay (UMG).
 * Shows speed and jetpack status.
 * Toggled with the tilde (`) key; not shown to players in shipping builds.
 */
UCLASS()
class FEDERATION_API UDevDiagnosticsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void BuildWidgetTree();

	UPROPERTY()
	TObjectPtr<UTextBlock> SpeedText;

	UPROPERTY()
	TObjectPtr<UTextBlock> JetpackEnabledText;

	UPROPERTY()
	TObjectPtr<UTextBlock> JetpackBoostText;
};
