// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemTypes.generated.h"

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Weapon,
	Armor,
	Shield,
	Ability,
	Biomorph,
	Consumable,
	Ammunition,
	Grenade,
	Software,
	QuestItem,
	Miscellaneous
};

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	Head,
	Body,
	PrimaryWeapon,
	SecondaryWeapon,
	Shield,
	Ability1,
	Ability2,
	Biomorph1,
	Biomorph2,
	Biomorph3
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Energy,
	Kinetic,
	Plasma,
	Fire,
	Explosive,
	Railgun,
	Assorted,
	Pryzct
};

UENUM(BlueprintType)
enum class EWeaponClass : uint8
{
	Pistol,
	Rifle,
	Shotgun,
	Cannon,
	MeleeBladed,
	MeleeBlunt,
	Launcher,
	Special
};

UENUM(BlueprintType)
enum class EConsumableType : uint8
{
	Food,
	Drink,
	Alcohol,
	Drug,
	CombatDrug,
	Medical
};
