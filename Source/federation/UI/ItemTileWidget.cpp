// Copyright Federation Game. All Rights Reserved.

#include "UI/ItemTileWidget.h"
#include "UI/ItemDragDropOperation.h"
#include "Inventory/ItemBase.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Styling/CoreStyle.h"

namespace TileStyle
{
	static const FLinearColor TileBg(0.06f, 0.06f, 0.1f, 0.9f);
	static const FLinearColor TileBorder(0.2f, 0.5f, 0.7f, 0.6f);
	static const FSlateColor TextMain = FSlateColor(FLinearColor(0.92f, 0.95f, 1.f));
	static const FSlateColor TextDim = FSlateColor(FLinearColor(0.55f, 0.6f, 0.65f));
	static constexpr float TileSize = 80.f;
}

void UItemTileWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetVisibility(ESlateVisibility::Visible);
	BuildWidgetTree();
}

void UItemTileWidget::BuildWidgetTree()
{
	if (!WidgetTree) return;

	FSlateFontInfo NameFont = FCoreStyle::GetDefaultFontStyle("Regular", 10);
	FSlateFontInfo CountFont = FCoreStyle::GetDefaultFontStyle("Bold", 10);

	USizeBox* Root = WidgetTree->ConstructWidget<USizeBox>();
	Root->SetWidthOverride(TileStyle::TileSize);
	Root->SetHeightOverride(TileStyle::TileSize);
	WidgetTree->RootWidget = Root;

	TileBorder = WidgetTree->ConstructWidget<UBorder>();
	TileBorder->SetBrushColor(TileStyle::TileBg);
	TileBorder->SetPadding(FMargin(6.f, 4.f));
	Root->AddChild(TileBorder);

	UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
	TileBorder->AddChild(VBox);

	NameText = WidgetTree->ConstructWidget<UTextBlock>();
	NameText->SetText(FText::FromString(TEXT("Item")));
	NameText->SetColorAndOpacity(TileStyle::TextMain);
	NameText->SetFont(NameFont);
	NameText->SetAutoWrapText(true);
	UVerticalBoxSlot* NameSlot = VBox->AddChildToVerticalBox(NameText);
	NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	CountText = WidgetTree->ConstructWidget<UTextBlock>();
	CountText->SetText(FText::GetEmpty());
	CountText->SetColorAndOpacity(TileStyle::TextDim);
	CountText->SetFont(CountFont);
	VBox->AddChildToVerticalBox(CountText);
}

void UItemTileWidget::SetItem(UItemBase* InItem, int32 InCount)
{
	Item = InItem;
	Count = InCount;
	if (NameText && Item)
	{
		NameText->SetText(Item->DisplayName);
	}
	if (CountText)
	{
		CountText->SetText(Count > 1
			? FText::FromString(FString::Printf(TEXT("x%d"), Count))
			: FText::GetEmpty());
	}
}

FReply UItemTileWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && Item)
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	return FReply::Unhandled();
}

void UItemTileWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!Item) return;

	UItemDragDropOperation* DragOp = NewObject<UItemDragDropOperation>();
	DragOp->DraggedItem = Item;
	DragOp->Pivot = EDragPivot::CenterCenter;

	// Create a lightweight drag visual
	UItemTileWidget* DragVisual = CreateWidget<UItemTileWidget>(GetOwningPlayer());
	if (DragVisual)
	{
		DragVisual->SetItem(Item, Count);
	}
	DragOp->DefaultDragVisual = DragVisual;
	OutOperation = DragOp;
}
