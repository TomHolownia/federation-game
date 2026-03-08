// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDragDropOperation.generated.h"

class UItemBase;

/**
 * Carries a UItemBase pointer during a drag-and-drop operation
 * between inventory item tiles and equipment slots.
 */
UCLASS()
class FEDERATION_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UItemBase> DraggedItem;
};
