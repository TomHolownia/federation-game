// Copyright Federation Game. All Rights Reserved.

#include "UI/InventoryWidget.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemBase.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/Spacer.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"
#include "Styling/CoreStyle.h"

namespace InvUI
{
	static const FLinearColor PanelBg(0.02f, 0.02f, 0.05f, 0.92f);
	static const FLinearColor SectionBg(0.04f, 0.04f, 0.08f, 0.6f);
	static const FSlateColor Accent = FSlateColor(FLinearColor(0.3f, 0.8f, 1.0f));
	static const FSlateColor TextMain = FSlateColor(FLinearColor(0.92f, 0.95f, 1.f));
	static const FSlateColor TextDim = FSlateColor(FLinearColor(0.55f, 0.6f, 0.65f));
	static const FLinearColor WeightBarFill(0.3f, 0.8f, 1.0f, 1.0f);
	static const FLinearColor WeightBarBg(0.1f, 0.1f, 0.15f, 1.0f);

	static const TArray<EEquipmentSlot> AllSlots = {
		EEquipmentSlot::Head,
		EEquipmentSlot::Body,
		EEquipmentSlot::PrimaryWeapon,
		EEquipmentSlot::SecondaryWeapon,
		EEquipmentSlot::Shield,
		EEquipmentSlot::Ability1,
		EEquipmentSlot::Ability2,
		EEquipmentSlot::Biomorph1,
		EEquipmentSlot::Biomorph2,
		EEquipmentSlot::Biomorph3
	};
}

// ---------------------------------------------------------------------------

void UInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
}

void UInventoryWidget::NativeDestruct()
{
	if (InventoryComp)
	{
		InventoryComp->OnNativeInventoryChanged.RemoveAll(this);
	}
	Super::NativeDestruct();
}

// ---------------------------------------------------------------------------
// Widget tree
// ---------------------------------------------------------------------------

void UInventoryWidget::BuildWidgetTree()
{
	if (!WidgetTree) return;

	FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 18);
	FSlateFontInfo SectionFont = FCoreStyle::GetDefaultFontStyle("Bold", 13);
	FSlateFontInfo ContentFont = FCoreStyle::GetDefaultFontStyle("Regular", 12);

	// Root canvas (full screen)
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Root;

	// Size-constrained panel (centered)
	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>();
	SizeBox->SetMinDesiredWidth(820.f);
	SizeBox->SetMinDesiredHeight(560.f);

	UCanvasPanelSlot* SizeSlot = Root->AddChildToCanvas(SizeBox);
	SizeSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
	SizeSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	SizeSlot->SetAutoSize(true);

	// Background
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
	Background->SetBrushColor(InvUI::PanelBg);
	Background->SetPadding(FMargin(20.f, 16.f));
	SizeBox->AddChild(Background);

	UVerticalBox* OuterVBox = WidgetTree->ConstructWidget<UVerticalBox>();
	Background->AddChild(OuterVBox);

	// ── Header row: title + weight ──
	{
		UHorizontalBox* HeaderRow = WidgetTree->ConstructWidget<UHorizontalBox>();
		OuterVBox->AddChildToVerticalBox(HeaderRow);

		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
		Title->SetText(FText::FromString(TEXT("INVENTORY")));
		Title->SetColorAndOpacity(InvUI::Accent);
		Title->SetFont(TitleFont);
		UHorizontalBoxSlot* TitleSlot = HeaderRow->AddChildToHorizontalBox(Title);
		TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TitleSlot->SetVerticalAlignment(VAlign_Center);

		WeightText = WidgetTree->ConstructWidget<UTextBlock>();
		WeightText->SetText(FText::FromString(TEXT("0.0 / 100.0")));
		WeightText->SetColorAndOpacity(InvUI::TextDim);
		WeightText->SetFont(ContentFont);
		UHorizontalBoxSlot* WtSlot = HeaderRow->AddChildToHorizontalBox(WeightText);
		WtSlot->SetVerticalAlignment(VAlign_Center);
		WtSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));
	}

	// ── Weight bar ──
	{
		USpacer* Gap = WidgetTree->ConstructWidget<USpacer>();
		Gap->SetSize(FVector2D(0.f, 6.f));
		OuterVBox->AddChildToVerticalBox(Gap);

		WeightBar = WidgetTree->ConstructWidget<UProgressBar>();
		WeightBar->SetPercent(0.f);
		WeightBar->SetFillColorAndOpacity(InvUI::WeightBarFill);
		OuterVBox->AddChildToVerticalBox(WeightBar);

		USpacer* Gap2 = WidgetTree->ConstructWidget<USpacer>();
		Gap2->SetSize(FVector2D(0.f, 12.f));
		OuterVBox->AddChildToVerticalBox(Gap2);
	}

	// ── Two-column body: Items | Equipment ──
	UHorizontalBox* Columns = WidgetTree->ConstructWidget<UHorizontalBox>();
	UVerticalBoxSlot* ColSlot = OuterVBox->AddChildToVerticalBox(Columns);
	ColSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	// Left column: Items
	{
		UBorder* ItemSection = WidgetTree->ConstructWidget<UBorder>();
		ItemSection->SetBrushColor(InvUI::SectionBg);
		ItemSection->SetPadding(FMargin(12.f, 8.f));
		UHorizontalBoxSlot* ItemColSlot = Columns->AddChildToHorizontalBox(ItemSection);
		ItemColSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		ItemColSlot->SetPadding(FMargin(0.f, 0.f, 6.f, 0.f));

		UVerticalBox* ItemVBox = WidgetTree->ConstructWidget<UVerticalBox>();
		ItemSection->AddChild(ItemVBox);

		UTextBlock* ItemsTitle = WidgetTree->ConstructWidget<UTextBlock>();
		ItemsTitle->SetText(FText::FromString(TEXT("ITEMS")));
		ItemsTitle->SetColorAndOpacity(InvUI::Accent);
		ItemsTitle->SetFont(SectionFont);
		ItemVBox->AddChildToVerticalBox(ItemsTitle);

		USpacer* Gap = WidgetTree->ConstructWidget<USpacer>();
		Gap->SetSize(FVector2D(0.f, 6.f));
		ItemVBox->AddChildToVerticalBox(Gap);

		UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
		UVerticalBoxSlot* ScrollSlot = ItemVBox->AddChildToVerticalBox(Scroll);
		ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		ItemListBox = WidgetTree->ConstructWidget<UVerticalBox>();
		Scroll->AddChild(ItemListBox);
	}

	// Right column: Equipment
	{
		UBorder* EquipSection = WidgetTree->ConstructWidget<UBorder>();
		EquipSection->SetBrushColor(InvUI::SectionBg);
		EquipSection->SetPadding(FMargin(12.f, 8.f));
		UHorizontalBoxSlot* EquipColSlot = Columns->AddChildToHorizontalBox(EquipSection);
		EquipColSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		EquipColSlot->SetPadding(FMargin(6.f, 0.f, 0.f, 0.f));

		UVerticalBox* EquipVBox = WidgetTree->ConstructWidget<UVerticalBox>();
		EquipSection->AddChild(EquipVBox);

		UTextBlock* EquipTitle = WidgetTree->ConstructWidget<UTextBlock>();
		EquipTitle->SetText(FText::FromString(TEXT("EQUIPMENT")));
		EquipTitle->SetColorAndOpacity(InvUI::Accent);
		EquipTitle->SetFont(SectionFont);
		EquipVBox->AddChildToVerticalBox(EquipTitle);

		USpacer* Gap = WidgetTree->ConstructWidget<USpacer>();
		Gap->SetSize(FVector2D(0.f, 6.f));
		EquipVBox->AddChildToVerticalBox(Gap);

		for (EEquipmentSlot EquipSlot : InvUI::AllSlots)
		{
			UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
			UVerticalBoxSlot* RowSlot = EquipVBox->AddChildToVerticalBox(Row);
			RowSlot->SetPadding(FMargin(0.f, 2.f));

			UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
			Label->SetText(GetSlotDisplayName(EquipSlot));
			Label->SetColorAndOpacity(InvUI::TextDim);
			Label->SetFont(ContentFont);
			Label->SetMinDesiredWidth(90.f);
			UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(Label);
			LabelSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));

			UTextBlock* Value = WidgetTree->ConstructWidget<UTextBlock>();
			Value->SetText(FText::FromString(TEXT("Empty")));
			Value->SetColorAndOpacity(InvUI::TextMain);
			Value->SetFont(ContentFont);
			Row->AddChildToHorizontalBox(Value);

			EquipmentSlotTexts.Add(EquipSlot, Value);
		}
	}
}

// ---------------------------------------------------------------------------
// Data binding
// ---------------------------------------------------------------------------

void UInventoryWidget::SetInventoryComponent(UInventoryComponent* InComp)
{
	if (InventoryComp)
	{
		InventoryComp->OnNativeInventoryChanged.RemoveAll(this);
	}
	InventoryComp = InComp;
	if (InventoryComp)
	{
		InventoryComp->OnNativeInventoryChanged.AddUObject(this, &UInventoryWidget::RefreshInventory);
	}
	RefreshInventory();
}

void UInventoryWidget::RefreshInventory()
{
	PopulateItems();
	PopulateEquipment();
	UpdateWeightBar();
}

// ---------------------------------------------------------------------------
// Populate helpers
// ---------------------------------------------------------------------------

void UInventoryWidget::PopulateItems()
{
	if (!ItemListBox) return;
	ItemListBox->ClearChildren();

	FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Regular", 11);

	if (!InventoryComp || InventoryComp->GetItems().Num() == 0)
	{
		UTextBlock* Empty = NewObject<UTextBlock>(this);
		Empty->SetText(FText::FromString(TEXT("No items")));
		Empty->SetColorAndOpacity(InvUI::TextDim);
		Empty->SetFont(Font);
		ItemListBox->AddChild(Empty);
		return;
	}

	for (const FInventoryEntry& Entry : InventoryComp->GetItems())
	{
		if (!Entry.ItemDef) continue;

		UHorizontalBox* Row = NewObject<UHorizontalBox>(this);

		// Name
		UTextBlock* Name = NewObject<UTextBlock>(this);
		Name->SetText(Entry.ItemDef->DisplayName);
		Name->SetColorAndOpacity(InvUI::TextMain);
		Name->SetFont(Font);
		UHorizontalBoxSlot* NameSlot = Cast<UHorizontalBoxSlot>(Row->AddChild(Name));
		if (NameSlot)
		{
			NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			NameSlot->SetPadding(FMargin(0.f, 2.f, 12.f, 2.f));
		}

		// Count
		UTextBlock* Count = NewObject<UTextBlock>(this);
		Count->SetText(FText::FromString(FString::Printf(TEXT("x%d"), Entry.Count)));
		Count->SetColorAndOpacity(InvUI::TextDim);
		Count->SetFont(Font);
		UHorizontalBoxSlot* CountSlot = Cast<UHorizontalBoxSlot>(Row->AddChild(Count));
		if (CountSlot)
		{
			CountSlot->SetPadding(FMargin(0.f, 2.f, 12.f, 2.f));
		}

		// Weight
		UTextBlock* Weight = NewObject<UTextBlock>(this);
		Weight->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Entry.ItemDef->Weight * Entry.Count)));
		Weight->SetColorAndOpacity(InvUI::TextDim);
		Weight->SetFont(Font);
		Row->AddChild(Weight);

		ItemListBox->AddChild(Row);
	}
}

void UInventoryWidget::PopulateEquipment()
{
	for (auto& Pair : EquipmentSlotTexts)
	{
		if (!Pair.Value) continue;
		UItemBase* Item = InventoryComp ? InventoryComp->GetEquippedItem(Pair.Key) : nullptr;
		if (Item)
		{
			Pair.Value->SetText(Item->DisplayName);
			Pair.Value->SetColorAndOpacity(InvUI::TextMain);
		}
		else
		{
			Pair.Value->SetText(FText::FromString(TEXT("Empty")));
			Pair.Value->SetColorAndOpacity(InvUI::TextDim);
		}
	}
}

void UInventoryWidget::UpdateWeightBar()
{
	const float Current = InventoryComp ? InventoryComp->GetCurrentWeight() : 0.f;
	const float Max = InventoryComp ? InventoryComp->MaxCarryWeight : 100.f;

	if (WeightBar)
	{
		WeightBar->SetPercent(Max > 0.f ? FMath::Clamp(Current / Max, 0.f, 1.f) : 0.f);
	}
	if (WeightText)
	{
		WeightText->SetText(FText::FromString(FString::Printf(TEXT("%.1f / %.1f"), Current, Max)));
	}
}

// ---------------------------------------------------------------------------

FText UInventoryWidget::GetSlotDisplayName(EEquipmentSlot Slot)
{
	switch (Slot)
	{
		case EEquipmentSlot::Head:            return FText::FromString(TEXT("Head"));
		case EEquipmentSlot::Body:            return FText::FromString(TEXT("Body"));
		case EEquipmentSlot::PrimaryWeapon:   return FText::FromString(TEXT("Primary"));
		case EEquipmentSlot::SecondaryWeapon: return FText::FromString(TEXT("Secondary"));
		case EEquipmentSlot::Shield:          return FText::FromString(TEXT("Shield"));
		case EEquipmentSlot::Ability1:        return FText::FromString(TEXT("Ability 1"));
		case EEquipmentSlot::Ability2:        return FText::FromString(TEXT("Ability 2"));
		case EEquipmentSlot::Biomorph1:       return FText::FromString(TEXT("Bio 1"));
		case EEquipmentSlot::Biomorph2:       return FText::FromString(TEXT("Bio 2"));
		case EEquipmentSlot::Biomorph3:       return FText::FromString(TEXT("Bio 3"));
		default:                              return FText::FromString(TEXT("Unknown"));
	}
}
