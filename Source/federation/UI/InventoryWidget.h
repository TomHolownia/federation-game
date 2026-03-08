// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/ItemTypes.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UTextBlock;
class UVerticalBox;
class UProgressBar;

/**
 * Player-facing inventory panel (UMG).
 * Shows carried items, equipped gear, and weight.
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
	void PopulateItems();
	void PopulateEquipment();
	void UpdateWeightBar();

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY()
	TObjectPtr<UVerticalBox> ItemListBox;

	UPROPERTY()
	TObjectPtr<UProgressBar> WeightBar;

	UPROPERTY()
	TObjectPtr<UTextBlock> WeightText;

	UPROPERTY()
	TMap<EEquipmentSlot, TObjectPtr<UTextBlock>> EquipmentSlotTexts;
};
