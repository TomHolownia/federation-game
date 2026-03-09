// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemBase.h"
#include "WeaponItem.generated.h"

/**
 * Data asset for weapon items.
 * Covers energy, kinetic, plasma, fire, explosive, railgun, assorted, and Pryzct weapons.
 */
UCLASS(Blueprintable, BlueprintType)
class FEDERATION_API UWeaponItem : public UItemBase
{
	GENERATED_BODY()

public:
	UWeaponItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::Kinetic;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponClass WeaponClass = EWeaponClass::Pistol;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "1", ClampMax = "2"))
	int32 HandsRequired = 1;
};
