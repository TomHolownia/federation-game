// Copyright Federation Game. All Rights Reserved.

#include "UI/EquipmentSlotWidget.h"
#include "UI/ItemDragDropOperation.h"
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

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDropOperation* ItemDrag = Cast<UItemDragDropOperation>(InOperation);
	if (!ItemDrag || !ItemDrag->DraggedItem || !InventoryComp) return false;

	UItemBase* DraggedItem = ItemDrag->DraggedItem;

	// Check slot compatibility: weapons go to weapon slots, equipment uses its Slot property
	bool bCompatible = false;
	if (UWeaponItem* Weapon = Cast<UWeaponItem>(DraggedItem))
	{
		bCompatible = (EquipSlot == EEquipmentSlot::PrimaryWeapon || EquipSlot == EEquipmentSlot::SecondaryWeapon);
	}
	else if (UEquipmentItem* Equipment = Cast<UEquipmentItem>(DraggedItem))
	{
		bCompatible = (Equipment->Slot == EquipSlot);
	}

	if (!bCompatible) return false;

	// Unequip current item in this slot first
	InventoryComp->UnequipSlot(EquipSlot);

	InventoryComp->EquipItem(DraggedItem);
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
			InventoryComp->UnequipSlot(EquipSlot);
			OnSlotChanged.ExecuteIfBound();
			return FReply::Handled();
		}
	}
	return FReply::Unhandled();
}
