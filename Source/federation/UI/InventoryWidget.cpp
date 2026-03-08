// Copyright Federation Game. All Rights Reserved.

#include "UI/InventoryWidget.h"
#include "UI/ItemTileWidget.h"
#include "UI/EquipmentSlotWidget.h"
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
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
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

	// Spatial positions for equipment slots (column, row) → pixel offset.
	// Layout mirrors a character silhouette per the design sketch.
	struct FSlotPos { EEquipmentSlot Slot; float X; float Y; };
	static const FSlotPos SlotPositions[] = {
		{ EEquipmentSlot::Head,            86.f,   0.f },
		{ EEquipmentSlot::SecondaryWeapon,  0.f,  86.f },
		{ EEquipmentSlot::Body,            86.f,  86.f },
		{ EEquipmentSlot::PrimaryWeapon,  172.f,  86.f },
		{ EEquipmentSlot::Shield,           0.f, 172.f },
		{ EEquipmentSlot::Ability1,        86.f, 172.f },
		{ EEquipmentSlot::Ability2,       172.f, 172.f },
		{ EEquipmentSlot::Biomorph1,        0.f, 258.f },
		{ EEquipmentSlot::Biomorph2,       86.f, 258.f },
		{ EEquipmentSlot::Biomorph3,      172.f, 258.f },
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
	FSlateFontInfo SmallFont = FCoreStyle::GetDefaultFontStyle("Regular", 12);

	// Root canvas (full screen)
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Root;

	// Size-constrained panel (centered)
	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>();
	SizeBox->SetMinDesiredWidth(920.f);
	SizeBox->SetMinDesiredHeight(620.f);

	UCanvasPanelSlot* SizeSlot = Root->AddChildToCanvas(SizeBox);
	SizeSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
	SizeSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	SizeSlot->SetAutoSize(true);

	// Background
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
	Background->SetBrushColor(InvUI::PanelBg);
	Background->SetPadding(FMargin(24.f, 18.f));
	SizeBox->AddChild(Background);

	UVerticalBox* OuterVBox = WidgetTree->ConstructWidget<UVerticalBox>();
	Background->AddChild(OuterVBox);

	// ── Header row: title + "Weight: X / Y" ──
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

		UTextBlock* WeightLabel = WidgetTree->ConstructWidget<UTextBlock>();
		WeightLabel->SetText(FText::FromString(TEXT("Weight:")));
		WeightLabel->SetColorAndOpacity(InvUI::Accent);
		WeightLabel->SetFont(SmallFont);
		UHorizontalBoxSlot* WlSlot = HeaderRow->AddChildToHorizontalBox(WeightLabel);
		WlSlot->SetVerticalAlignment(VAlign_Center);
		WlSlot->SetPadding(FMargin(8.f, 0.f, 6.f, 0.f));

		WeightText = WidgetTree->ConstructWidget<UTextBlock>();
		WeightText->SetText(FText::FromString(TEXT("0.0 / 100.0")));
		WeightText->SetColorAndOpacity(InvUI::TextDim);
		WeightText->SetFont(SmallFont);
		UHorizontalBoxSlot* WtSlot = HeaderRow->AddChildToHorizontalBox(WeightText);
		WtSlot->SetVerticalAlignment(VAlign_Center);
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
		Gap2->SetSize(FVector2D(0.f, 14.f));
		OuterVBox->AddChildToVerticalBox(Gap2);
	}

	// ── Two-column body: Items (tile grid) | Equipped (spatial slots) ──
	UHorizontalBox* Columns = WidgetTree->ConstructWidget<UHorizontalBox>();
	UVerticalBoxSlot* ColSlot = OuterVBox->AddChildToVerticalBox(Columns);
	ColSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	// Left column: Items tile grid
	{
		UBorder* ItemSection = WidgetTree->ConstructWidget<UBorder>();
		ItemSection->SetBrushColor(InvUI::SectionBg);
		ItemSection->SetPadding(FMargin(12.f, 8.f));
		UHorizontalBoxSlot* ItemColSlot = Columns->AddChildToHorizontalBox(ItemSection);
		ItemColSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		ItemColSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));

		UVerticalBox* ItemVBox = WidgetTree->ConstructWidget<UVerticalBox>();
		ItemSection->AddChild(ItemVBox);

		UTextBlock* ItemsTitle = WidgetTree->ConstructWidget<UTextBlock>();
		ItemsTitle->SetText(FText::FromString(TEXT("ITEMS")));
		ItemsTitle->SetColorAndOpacity(InvUI::Accent);
		ItemsTitle->SetFont(SectionFont);
		ItemVBox->AddChildToVerticalBox(ItemsTitle);

		USpacer* Gap = WidgetTree->ConstructWidget<USpacer>();
		Gap->SetSize(FVector2D(0.f, 8.f));
		ItemVBox->AddChildToVerticalBox(Gap);

		UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
		UVerticalBoxSlot* ScrollSlot = ItemVBox->AddChildToVerticalBox(Scroll);
		ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		ItemGrid = WidgetTree->ConstructWidget<UWrapBox>();
		ItemGrid->SetInnerSlotPadding(FVector2D(4.f, 4.f));
		Scroll->AddChild(ItemGrid);
	}

	// Right column: Equipped — spatial slot layout
	{
		UBorder* EquipSection = WidgetTree->ConstructWidget<UBorder>();
		EquipSection->SetBrushColor(InvUI::SectionBg);
		EquipSection->SetPadding(FMargin(12.f, 8.f));
		UHorizontalBoxSlot* EquipColSlot = Columns->AddChildToHorizontalBox(EquipSection);
		EquipColSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));

		UVerticalBox* EquipVBox = WidgetTree->ConstructWidget<UVerticalBox>();
		EquipSection->AddChild(EquipVBox);

		UTextBlock* EquipTitle = WidgetTree->ConstructWidget<UTextBlock>();
		EquipTitle->SetText(FText::FromString(TEXT("EQUIPPED")));
		EquipTitle->SetColorAndOpacity(InvUI::Accent);
		EquipTitle->SetFont(SectionFont);
		EquipVBox->AddChildToVerticalBox(EquipTitle);

		USpacer* Gap = WidgetTree->ConstructWidget<USpacer>();
		Gap->SetSize(FVector2D(0.f, 8.f));
		EquipVBox->AddChildToVerticalBox(Gap);

		// Canvas panel for spatial slot positioning
		UCanvasPanel* SlotCanvas = WidgetTree->ConstructWidget<UCanvasPanel>();
		UVerticalBoxSlot* CanvasSlot = EquipVBox->AddChildToVerticalBox(SlotCanvas);
		CanvasSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		// Equipment slot widgets are created dynamically (not via WidgetTree)
		// because they are UUserWidget subclasses that need CreateWidget<>().
		// We defer creation to SetInventoryComponent / first refresh.
		// Store the canvas for later use.
		SlotCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		// Pre-create empty slot outlines as placeholders
		for (const InvUI::FSlotPos& Pos : InvUI::SlotPositions)
		{
			UBorder* Placeholder = WidgetTree->ConstructWidget<UBorder>();
			Placeholder->SetBrushColor(FLinearColor(0.04f, 0.04f, 0.08f, 0.5f));

			USizeBox* PlaceholderSize = WidgetTree->ConstructWidget<USizeBox>();
			PlaceholderSize->SetWidthOverride(80.f);
			PlaceholderSize->SetHeightOverride(80.f);
			PlaceholderSize->AddChild(Placeholder);

			UCanvasPanelSlot* PSlot = SlotCanvas->AddChildToCanvas(PlaceholderSize);
			PSlot->SetPosition(FVector2D(Pos.X, Pos.Y));
			PSlot->SetAutoSize(true);
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
	PopulateItemTiles();
	RefreshEquipmentSlots();
	UpdateWeightBar();
}

// ---------------------------------------------------------------------------
// Item tile grid
// ---------------------------------------------------------------------------

void UInventoryWidget::PopulateItemTiles()
{
	if (!ItemGrid) return;
	ItemGrid->ClearChildren();

	if (!InventoryComp || InventoryComp->GetItems().Num() == 0)
	{
		UTextBlock* Empty = NewObject<UTextBlock>(this);
		Empty->SetText(FText::FromString(TEXT("No items")));
		Empty->SetColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.6f, 0.65f)));
		Empty->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 11));
		ItemGrid->AddChild(Empty);
		return;
	}

	for (const FInventoryEntry& Entry : InventoryComp->GetItems())
	{
		if (!Entry.ItemDef) continue;

		UItemTileWidget* Tile = CreateWidget<UItemTileWidget>(GetOwningPlayer());
		if (Tile)
		{
			Tile->SetItem(Entry.ItemDef, Entry.Count);
			ItemGrid->AddChild(Tile);
		}
	}
}

// ---------------------------------------------------------------------------
// Equipment slots
// ---------------------------------------------------------------------------

void UInventoryWidget::RefreshEquipmentSlots()
{
	// Create slot widgets on first call (needs CreateWidget which requires a player controller)
	if (EquipmentSlots.Num() == 0 && GetOwningPlayer())
	{
		// Find the slot canvas — it's inside the equipment section
		// We need to add the slot widgets to the same canvas that has the placeholders.
		// The canvas is the last canvas panel we created in BuildWidgetTree.
		// Navigate: Root → SizeBox → Background → OuterVBox → Columns → EquipSection → EquipVBox → Canvas
		UCanvasPanel* SlotCanvas = nullptr;

		if (WidgetTree && WidgetTree->RootWidget)
		{
			// Walk the tree to find our slot canvas
			TArray<UWidget*> AllWidgets;
			WidgetTree->GetAllWidgets(AllWidgets);
			for (UWidget* W : AllWidgets)
			{
				UCanvasPanel* Canvas = Cast<UCanvasPanel>(W);
				if (Canvas && Canvas != WidgetTree->RootWidget)
				{
					SlotCanvas = Canvas;
				}
			}
		}

		if (SlotCanvas)
		{
			for (const InvUI::FSlotPos& Pos : InvUI::SlotPositions)
			{
				UEquipmentSlotWidget* SlotWidget = CreateWidget<UEquipmentSlotWidget>(GetOwningPlayer());
				if (!SlotWidget) continue;

				SlotWidget->Configure(Pos.Slot, InventoryComp);
				SlotWidget->OnSlotChanged.BindUObject(this, &UInventoryWidget::RefreshInventory);

				UCanvasPanelSlot* CSlot = SlotCanvas->AddChildToCanvas(SlotWidget);
				CSlot->SetPosition(FVector2D(Pos.X, Pos.Y));
				CSlot->SetAutoSize(true);

				EquipmentSlots.Add(SlotWidget);
			}
		}
	}

	// Refresh existing slot widgets
	for (UEquipmentSlotWidget* SlotWidget : EquipmentSlots)
	{
		if (SlotWidget)
		{
			SlotWidget->Refresh();
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

FText UInventoryWidget::GetSlotDisplayName(EEquipmentSlot InSlot)
{
	switch (InSlot)
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
