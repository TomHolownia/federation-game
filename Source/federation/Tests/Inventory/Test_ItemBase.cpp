// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Inventory/ItemBase.h"
#include "Inventory/WeaponItem.h"
#include "Inventory/EquipmentItem.h"
#include "Inventory/ConsumableItem.h"

#if WITH_DEV_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// UItemBase defaults
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemBaseDefaultCategory,
	"FederationGame.Inventory.ItemBase.DefaultCategoryIsMiscellaneous",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FItemBaseDefaultCategory::RunTest(const FString& Parameters)
{
	UItemBase* Item = NewObject<UItemBase>();
	TestEqual(TEXT("Default category"), Item->Category, EItemCategory::Miscellaneous);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemBaseDefaultWeight,
	"FederationGame.Inventory.ItemBase.DefaultWeightIsZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FItemBaseDefaultWeight::RunTest(const FString& Parameters)
{
	UItemBase* Item = NewObject<UItemBase>();
	TestEqual(TEXT("Default weight"), Item->Weight, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemBaseDefaultStackSize,
	"FederationGame.Inventory.ItemBase.DefaultMaxStackSizeIsOne",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FItemBaseDefaultStackSize::RunTest(const FString& Parameters)
{
	UItemBase* Item = NewObject<UItemBase>();
	TestEqual(TEXT("Default max stack size"), Item->MaxStackSize, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemBaseNotQuestItemByDefault,
	"FederationGame.Inventory.ItemBase.NotQuestItemByDefault",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FItemBaseNotQuestItemByDefault::RunTest(const FString& Parameters)
{
	UItemBase* Item = NewObject<UItemBase>();
	TestFalse(TEXT("bIsQuestItem should default to false"), Item->bIsQuestItem);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemBasePrimaryAssetId,
	"FederationGame.Inventory.ItemBase.PrimaryAssetIdUsesItemID",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FItemBasePrimaryAssetId::RunTest(const FString& Parameters)
{
	UItemBase* Item = NewObject<UItemBase>();
	Item->ItemID = FName("TestItem");
	FPrimaryAssetId Id = Item->GetPrimaryAssetId();
	TestEqual(TEXT("Asset type"), Id.PrimaryAssetType.GetName(), FName("Item"));
	TestEqual(TEXT("Asset name"), Id.PrimaryAssetName, FName("TestItem"));
	return true;
}

// ---------------------------------------------------------------------------
// UWeaponItem
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponItemDefaultCategory,
	"FederationGame.Inventory.WeaponItem.CategoryIsWeapon",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWeaponItemDefaultCategory::RunTest(const FString& Parameters)
{
	UWeaponItem* Weapon = NewObject<UWeaponItem>();
	TestEqual(TEXT("Weapon category"), Weapon->Category, EItemCategory::Weapon);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponItemDefaultHands,
	"FederationGame.Inventory.WeaponItem.DefaultHandsIsOne",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWeaponItemDefaultHands::RunTest(const FString& Parameters)
{
	UWeaponItem* Weapon = NewObject<UWeaponItem>();
	TestEqual(TEXT("Default hands required"), Weapon->HandsRequired, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponItemNotStackable,
	"FederationGame.Inventory.WeaponItem.NotStackable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWeaponItemNotStackable::RunTest(const FString& Parameters)
{
	UWeaponItem* Weapon = NewObject<UWeaponItem>();
	TestEqual(TEXT("Weapons should not stack"), Weapon->MaxStackSize, 1);
	return true;
}

// ---------------------------------------------------------------------------
// UEquipmentItem
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEquipmentItemDefaultCategory,
	"FederationGame.Inventory.EquipmentItem.DefaultCategoryIsArmor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FEquipmentItemDefaultCategory::RunTest(const FString& Parameters)
{
	UEquipmentItem* Equip = NewObject<UEquipmentItem>();
	TestEqual(TEXT("Equipment default category"), Equip->Category, EItemCategory::Armor);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEquipmentItemDefaultSlot,
	"FederationGame.Inventory.EquipmentItem.DefaultSlotIsBody",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FEquipmentItemDefaultSlot::RunTest(const FString& Parameters)
{
	UEquipmentItem* Equip = NewObject<UEquipmentItem>();
	TestEqual(TEXT("Equipment default slot"), Equip->Slot, EEquipmentSlot::Body);
	return true;
}

// ---------------------------------------------------------------------------
// UConsumableItem
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FConsumableItemDefaultCategory,
	"FederationGame.Inventory.ConsumableItem.DefaultCategoryIsConsumable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FConsumableItemDefaultCategory::RunTest(const FString& Parameters)
{
	UConsumableItem* Food = NewObject<UConsumableItem>();
	TestEqual(TEXT("Consumable default category"), Food->Category, EItemCategory::Consumable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FConsumableItemDefaultDuration,
	"FederationGame.Inventory.ConsumableItem.DefaultDurationIsZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FConsumableItemDefaultDuration::RunTest(const FString& Parameters)
{
	UConsumableItem* Food = NewObject<UConsumableItem>();
	TestEqual(TEXT("Consumable default duration"), Food->Duration, 0.f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
