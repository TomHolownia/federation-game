// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemTileWidget.generated.h"

class UItemBase;
class UTextBlock;
class UBorder;

/**
 * Draggable inventory item tile.
 * Displays item name and stack count. Can be dragged to equipment slots.
 */
UCLASS()
class FEDERATION_API UItemTileWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetItem(UItemBase* InItem, int32 InCount);

	UPROPERTY()
	TObjectPtr<UItemBase> Item;

	int32 Count = 0;

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

private:
	void BuildWidgetTree();

	UPROPERTY()
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY()
	TObjectPtr<UTextBlock> CountText;

	UPROPERTY()
	TObjectPtr<UBorder> TileBorder;
};
