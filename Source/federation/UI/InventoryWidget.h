// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/ItemTypes.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UTextBlock;
class UProgressBar;
class UWrapBox;
class UEquipmentSlotWidget;

/**
 * Player-facing inventory panel (UMG).
 * Left side: draggable item tiles in a grid.
 * Right side: spatial equipment slots matching a character silhouette layout.
 * Toggled with Tab; shows mouse cursor while open.
 */
UCLASS()
class FEDERATION_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	void SetInventoryComponent(UInventoryComponent* InComp);
	bool HasInventoryComponent() const { return InventoryComp != nullptr; }

	UFUNCTION()
	void RefreshInventory();

	static FText GetSlotDisplayName(EEquipmentSlot Slot);

private:
	void BuildWidgetTree();
	void PopulateItemTiles();
	void RefreshEquipmentSlots();
	void UpdateWeightBar();

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY()
	TObjectPtr<UWrapBox> ItemGrid;

	UPROPERTY()
	TObjectPtr<UProgressBar> WeightBar;

	UPROPERTY()
	TObjectPtr<UTextBlock> WeightText;

	UPROPERTY()
	TArray<TObjectPtr<UEquipmentSlotWidget>> EquipmentSlots;
};
