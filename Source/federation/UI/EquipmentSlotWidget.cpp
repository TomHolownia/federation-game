// Copyright Federation Game. All Rights Reserved.

#include "UI/EquipmentSlotWidget.h"
#include "UI/ItemDragDropOperation.h"
#include "UI/ItemTileWidget.h"
#include "UI/InventoryWidget.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemBase.h"
#include "Inventory/WeaponItem.h"
#include "Inventory/EquipmentItem.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"
#include "Styling/CoreStyle.h"

namespace SlotStyle
{
	static const FLinearColor EmptyBg(0.04f, 0.04f, 0.08f, 0.8f);
	static const FLinearColor OccupiedBg(0.08f, 0.12f, 0.18f, 0.9f);
	static const FSlateColor LabelColor = FSlateColor(FLinearColor(0.4f, 0.55f, 0.65f));
	static const FSlateColor ItemColor = FSlateColor(FLinearColor(0.92f, 0.95f, 1.f));
	static const FSlateColor EmptyColor = FSlateColor(FLinearColor(0.35f, 0.38f, 0.42f));
	static constexpr float SlotSize = 80.f;
}

void UEquipmentSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetVisibility(ESlateVisibility::Visible);
	BuildWidgetTree();
}

void UEquipmentSlotWidget::BuildWidgetTree()
{
	if (!WidgetTree) return;

	FSlateFontInfo LabelFont = FCoreStyle::GetDefaultFontStyle("Bold", 8);
	FSlateFontInfo ItemFont = FCoreStyle::GetDefaultFontStyle("Regular", 9);

	USizeBox* Root = WidgetTree->ConstructWidget<USizeBox>();
	Root->SetWidthOverride(SlotStyle::SlotSize);
	Root->SetHeightOverride(SlotStyle::SlotSize);
	WidgetTree->RootWidget = Root;

	SlotBorder = WidgetTree->ConstructWidget<UBorder>();
	SlotBorder->SetBrushColor(SlotStyle::EmptyBg);
	SlotBorder->SetPadding(FMargin(5.f, 3.f));
	Root->AddChild(SlotBorder);

	UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
	SlotBorder->AddChild(VBox);

	SlotLabel = WidgetTree->ConstructWidget<UTextBlock>();
	SlotLabel->SetText(FText::FromString(TEXT("SLOT")));
	SlotLabel->SetColorAndOpacity(SlotStyle::LabelColor);
	SlotLabel->SetFont(LabelFont);
	VBox->AddChildToVerticalBox(SlotLabel);

	ItemNameText = WidgetTree->ConstructWidget<UTextBlock>();
	ItemNameText->SetText(FText::FromString(TEXT("Empty")));
	ItemNameText->SetColorAndOpacity(SlotStyle::EmptyColor);
	ItemNameText->SetFont(ItemFont);
	ItemNameText->SetAutoWrapText(true);
	UVerticalBoxSlot* ItemSlot = VBox->AddChildToVerticalBox(ItemNameText);
	ItemSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	ItemSlot->SetVerticalAlignment(VAlign_Center);
}

void UEquipmentSlotWidget::Configure(EEquipmentSlot InSlot, UInventoryComponent* InComp)
{
	EquipSlot = InSlot;
	InventoryComp = InComp;
	if (SlotLabel)
	{
		SlotLabel->SetText(UInventoryWidget::GetSlotDisplayName(InSlot));
	}
	Refresh();
}

void UEquipmentSlotWidget::Refresh()
{
	UItemBase* Equipped = InventoryComp ? InventoryComp->GetEquippedItem(EquipSlot) : nullptr;
	if (ItemNameText)
	{
		if (Equipped)
		{
			ItemNameText->SetText(Equipped->DisplayName);
			ItemNameText->SetColorAndOpacity(SlotStyle::ItemColor);
		}
		else
		{
			ItemNameText->SetText(FText::FromString(TEXT("Empty")));
			ItemNameText->SetColorAndOpacity(SlotStyle::EmptyColor);
		}
	}
	if (SlotBorder)
	{
		SlotBorder->SetBrushColor(Equipped ? SlotStyle::OccupiedBg : SlotStyle::EmptyBg);
	}
}

bool UEquipmentSlotWidget::IsItemCompatible(const UItemBase* Item) const
{
	if (const UWeaponItem* Weapon = Cast<UWeaponItem>(Item))
	{
		return EquipSlot == EEquipmentSlot::PrimaryWeapon || EquipSlot == EEquipmentSlot::SecondaryWeapon;
	}
	if (const UEquipmentItem* Equipment = Cast<UEquipmentItem>(Item))
	{
		return UInventoryComponent::AreSlotsFamilyCompatible(Equipment->Slot, EquipSlot);
	}
	return false;
}

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDropOperation* ItemDrag = Cast<UItemDragDropOperation>(InOperation);
	if (!ItemDrag || !ItemDrag->DraggedItem || !InventoryComp) return false;

	UItemBase* DraggedItem = ItemDrag->DraggedItem;
	if (!IsItemCompatible(DraggedItem)) return false;

	if (ItemDrag->bFromEquipment)
	{
		// Dragging from another equipment slot to this one.
		// Unequip source slot (returns item to Items array).
		InventoryComp->UnequipSlot(ItemDrag->SourceSlot);
	}

	// Unequip current occupant of this slot (returns to Items if occupied).
	InventoryComp->UnequipSlot(EquipSlot);

	// Equip the dragged item directly to this slot.
	InventoryComp->EquipItemToSlot(DraggedItem, EquipSlot);
	OnSlotChanged.ExecuteIfBound();
	return true;
}

FReply UEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && InventoryComp)
	{
		UItemBase* Equipped = InventoryComp->GetEquippedItem(EquipSlot);
		if (Equipped)
		{
			return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}
	}
	return FReply::Unhandled();
}

void UEquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	UItemBase* Equipped = InventoryComp ? InventoryComp->GetEquippedItem(EquipSlot) : nullptr;
	if (!Equipped) return;

	UItemDragDropOperation* DragOp = NewObject<UItemDragDropOperation>();
	DragOp->DraggedItem = Equipped;
	DragOp->SourceSlot = EquipSlot;
	DragOp->bFromEquipment = true;
	DragOp->Pivot = EDragPivot::CenterCenter;

	UItemTileWidget* DragVisual = CreateWidget<UItemTileWidget>(GetOwningPlayer());
	if (DragVisual)
	{
		DragVisual->SetItem(Equipped, 1);
	}
	DragOp->DefaultDragVisual = DragVisual;
	OutOperation = DragOp;
}
