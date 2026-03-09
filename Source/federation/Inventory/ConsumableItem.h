// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemBase.h"
#include "ConsumableItem.generated.h"

/**
 * Data asset for consumable items (food, drink, drugs, medical supplies).
 */
UCLASS(Blueprintable, BlueprintType)
class FEDERATION_API UConsumableItem : public UItemBase
{
	GENERATED_BODY()

public:
	UConsumableItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	EConsumableType ConsumableType = EConsumableType::Food;

	/** How long the effect lasts in seconds. 0 = instant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable", meta = (ClampMin = "0.0"))
	float Duration = 0.f;
};
