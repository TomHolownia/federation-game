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
 * Equipped items can be dragged to other slots or back to inventory.
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
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

private:
	void BuildWidgetTree();
	bool IsItemCompatible(const UItemBase* Item) const;

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
