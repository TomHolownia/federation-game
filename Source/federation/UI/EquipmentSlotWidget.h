// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/ItemTypes.h"
#include "EquipmentSlotWidget.generated.h"

class UItemBase;
class UInventoryComponent;
class UTextBlock;
class UBorder;

/**
 * A single equipment slot in the spatial equipment layout.
 * Accepts item drops and displays the currently equipped item.
 * Click an occupied slot to unequip.
 */
UCLASS()
class FEDERATION_API UEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Configure(EEquipmentSlot InSlot, UInventoryComponent* InComp);
	void Refresh();

	EEquipmentSlot GetEquipSlot() const { return EquipSlot; }

	DECLARE_DELEGATE(FOnSlotChanged);
	FOnSlotChanged OnSlotChanged;

protected:
	virtual void NativeOnInitialized() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void BuildWidgetTree();

	UPROPERTY()
	EEquipmentSlot EquipSlot = EEquipmentSlot::Body;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY()
	TObjectPtr<UTextBlock> SlotLabel;

	UPROPERTY()
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY()
	TObjectPtr<UBorder> SlotBorder;
};
