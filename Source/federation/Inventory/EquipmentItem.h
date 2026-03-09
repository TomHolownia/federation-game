// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemBase.h"
#include "EquipmentItem.generated.h"

/**
 * Data asset for equippable items (armor, shields, abilities, biomorphs).
 * The Slot determines which equipment slot this item occupies.
 */
UCLASS(Blueprintable, BlueprintType)
class FEDERATION_API UEquipmentItem : public UItemBase
{
	GENERATED_BODY()

public:
	UEquipmentItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	EEquipmentSlot Slot = EEquipmentSlot::Body;
};
