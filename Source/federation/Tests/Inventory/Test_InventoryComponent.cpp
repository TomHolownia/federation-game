// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemBase.h"
#include "Inventory/WeaponItem.h"
#include "Inventory/EquipmentItem.h"
#include "Inventory/ConsumableItem.h"
#include "Character/FederationCharacter.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
	UItemBase* MakeItem(float Weight = 1.f, int32 MaxStack = 1, bool bQuest = false)
	{
		UItemBase* Item = NewObject<UItemBase>();
		Item->Weight = Weight;
		Item->MaxStackSize = MaxStack;
		Item->bIsQuestItem = bQuest;
		return Item;
	}

	UInventoryComponent* MakeInventory(float MaxWeight = 100.f)
	{
		UInventoryComponent* Inv = NewObject<UInventoryComponent>();
		Inv->MaxCarryWeight = MaxWeight;
		return Inv;
	}
}

// ---------------------------------------------------------------------------
// Add / Remove
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryAddItem,
	"FederationGame.Inventory.Component.AddItem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryAddItem::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UItemBase* Item = MakeItem();
	TestTrue(TEXT("AddItem should succeed"), Inv->AddItem(Item));
	TestTrue(TEXT("HasItem should return true"), Inv->HasItem(Item));
	TestEqual(TEXT("Count should be 1"), Inv->GetItemCount(Item), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryAddMultiple,
	"FederationGame.Inventory.Component.AddMultipleCopies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryAddMultiple::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UItemBase* Item = MakeItem(1.f, 99);
	Inv->AddItem(Item, 5);
	TestEqual(TEXT("Count should be 5"), Inv->GetItemCount(Item), 5);
	Inv->AddItem(Item, 3);
	TestEqual(TEXT("Count should be 8"), Inv->GetItemCount(Item), 8);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryRemoveItem,
	"FederationGame.Inventory.Component.RemoveItem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryRemoveItem::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UItemBase* Item = MakeItem(1.f, 99);
	Inv->AddItem(Item, 5);
	TestTrue(TEXT("RemoveItem should succeed"), Inv->RemoveItem(Item, 3));
	TestEqual(TEXT("Count should be 2"), Inv->GetItemCount(Item), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryRemoveAll,
	"FederationGame.Inventory.Component.RemoveAllRemovesEntry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryRemoveAll::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UItemBase* Item = MakeItem();
	Inv->AddItem(Item);
	TestTrue(TEXT("Remove should succeed"), Inv->RemoveItem(Item, 1));
	TestFalse(TEXT("HasItem should return false"), Inv->HasItem(Item));
	TestEqual(TEXT("Items array should be empty"), Inv->Items.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryRemoveMoreThanOwned,
	"FederationGame.Inventory.Component.RemoveMoreThanOwnedFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryRemoveMoreThanOwned::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UItemBase* Item = MakeItem(1.f, 99);
	Inv->AddItem(Item, 2);
	TestFalse(TEXT("Cannot remove more than owned"), Inv->RemoveItem(Item, 5));
	TestEqual(TEXT("Count should remain 2"), Inv->GetItemCount(Item), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryAddNullFails,
	"FederationGame.Inventory.Component.AddNullFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryAddNullFails::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	TestFalse(TEXT("AddItem(nullptr) should fail"), Inv->AddItem(nullptr));
	return true;
}

// ---------------------------------------------------------------------------
// Weight system
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryWeightCalculation,
	"FederationGame.Inventory.Component.WeightCalculation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryWeightCalculation::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory(100.f);
	UItemBase* Heavy = MakeItem(10.f, 99);
	Inv->AddItem(Heavy, 3);
	TestEqual(TEXT("Weight should be 30"), Inv->GetCurrentWeight(), 30.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryWeightLimit,
	"FederationGame.Inventory.Component.WeightLimitPreventsAdd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryWeightLimit::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory(10.f);
	UItemBase* Heavy = MakeItem(5.f, 99);
	TestTrue(TEXT("First add should succeed"), Inv->AddItem(Heavy, 2));
	TestFalse(TEXT("Third item would exceed weight"), Inv->AddItem(Heavy, 1));
	TestEqual(TEXT("Count should remain 2"), Inv->GetItemCount(Heavy), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryQuestItemBypassesWeight,
	"FederationGame.Inventory.Component.QuestItemBypassesWeightLimit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryQuestItemBypassesWeight::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory(1.f);
	UItemBase* Quest = MakeItem(999.f, 99, true);
	TestTrue(TEXT("Quest item should bypass weight"), Inv->AddItem(Quest, 10));
	TestEqual(TEXT("Weight should be 0 (quest items excluded)"), Inv->GetCurrentWeight(), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryWeightIncludesEquipped,
	"FederationGame.Inventory.Component.WeightIncludesEquippedItems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryWeightIncludesEquipped::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UEquipmentItem* Helmet = NewObject<UEquipmentItem>();
	Helmet->Slot = EEquipmentSlot::Head;
	Helmet->Weight = 5.f;
	Inv->AddItem(Helmet);
	const float WeightBefore = Inv->GetCurrentWeight();

	Inv->EquipItem(Helmet);
	TestEqual(TEXT("Weight should be unchanged after equipping"), Inv->GetCurrentWeight(), WeightBefore);
	return true;
}

// ---------------------------------------------------------------------------
// Equip / Unequip
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipWeapon,
	"FederationGame.Inventory.Component.EquipWeaponToPrimarySlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryEquipWeapon::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UWeaponItem* Pistol = NewObject<UWeaponItem>();
	Pistol->Weight = 2.f;
	Inv->AddItem(Pistol);
	TestTrue(TEXT("Equip should succeed"), Inv->EquipItem(Pistol));
	TestEqual(TEXT("Primary slot should hold pistol"),
		Inv->GetEquippedItem(EEquipmentSlot::PrimaryWeapon), static_cast<UItemBase*>(Pistol));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipRemovesFromItems,
	"FederationGame.Inventory.Component.EquipRemovesFromItems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryEquipRemovesFromItems::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UEquipmentItem* Helmet = NewObject<UEquipmentItem>();
	Helmet->Slot = EEquipmentSlot::Head;
	Helmet->Weight = 2.f;
	Inv->AddItem(Helmet);
	TestTrue(TEXT("Item in inventory before equip"), Inv->HasItem(Helmet));

	Inv->EquipItem(Helmet);
	TestFalse(TEXT("Item removed from items after equip"), Inv->HasItem(Helmet));
	TestNotNull(TEXT("Item is in equipped slot"), Inv->GetEquippedItem(EEquipmentSlot::Head));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryUnequipRestoresToItems,
	"FederationGame.Inventory.Component.UnequipRestoresToItems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryUnequipRestoresToItems::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UEquipmentItem* Helmet = NewObject<UEquipmentItem>();
	Helmet->Slot = EEquipmentSlot::Head;
	Helmet->Weight = 2.f;
	Inv->AddItem(Helmet);
	Inv->EquipItem(Helmet);
	TestFalse(TEXT("Not in items while equipped"), Inv->HasItem(Helmet));

	Inv->UnequipSlot(EEquipmentSlot::Head);
	TestTrue(TEXT("Back in items after unequip"), Inv->HasItem(Helmet));
	TestNull(TEXT("Slot is empty after unequip"), Inv->GetEquippedItem(EEquipmentSlot::Head));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipSecondWeapon,
	"FederationGame.Inventory.Component.SecondWeaponGoesToSecondarySlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryEquipSecondWeapon::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UWeaponItem* Pistol = NewObject<UWeaponItem>();
	Pistol->Weight = 2.f;
	UWeaponItem* Rifle = NewObject<UWeaponItem>();
	Rifle->Weight = 4.f;
	Inv->AddItem(Pistol);
	Inv->AddItem(Rifle);
	Inv->EquipItem(Pistol);
	Inv->EquipItem(Rifle);
	TestEqual(TEXT("Primary should be pistol"),
		Inv->GetEquippedItem(EEquipmentSlot::PrimaryWeapon), static_cast<UItemBase*>(Pistol));
	TestEqual(TEXT("Secondary should be rifle"),
		Inv->GetEquippedItem(EEquipmentSlot::SecondaryWeapon), static_cast<UItemBase*>(Rifle));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipEquipment,
	"FederationGame.Inventory.Component.EquipEquipmentToCorrectSlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryEquipEquipment::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UEquipmentItem* Helmet = NewObject<UEquipmentItem>();
	Helmet->Slot = EEquipmentSlot::Head;
	Helmet->Weight = 1.f;
	Inv->AddItem(Helmet);
	TestTrue(TEXT("Equip should succeed"), Inv->EquipItem(Helmet));
	TestEqual(TEXT("Head slot should hold helmet"),
		Inv->GetEquippedItem(EEquipmentSlot::Head), static_cast<UItemBase*>(Helmet));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryUnequipSlot,
	"FederationGame.Inventory.Component.UnequipSlotReturnsItem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryUnequipSlot::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UWeaponItem* Pistol = NewObject<UWeaponItem>();
	Pistol->Weight = 2.f;
	Inv->AddItem(Pistol);
	Inv->EquipItem(Pistol);
	UItemBase* Removed = Inv->UnequipSlot(EEquipmentSlot::PrimaryWeapon);
	TestEqual(TEXT("Should return the pistol"), Removed, static_cast<UItemBase*>(Pistol));
	TestNull(TEXT("Slot should now be empty"), Inv->GetEquippedItem(EEquipmentSlot::PrimaryWeapon));
	TestTrue(TEXT("Pistol should be back in items"), Inv->HasItem(Pistol));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryUnequipEmptySlot,
	"FederationGame.Inventory.Component.UnequipEmptySlotReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryUnequipEmptySlot::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	TestNull(TEXT("Empty slot should return nullptr"), Inv->UnequipSlot(EEquipmentSlot::Head));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipRequiresOwnership,
	"FederationGame.Inventory.Component.EquipRequiresItemInInventory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryEquipRequiresOwnership::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UWeaponItem* Pistol = NewObject<UWeaponItem>();
	Pistol->Weight = 2.f;
	TestFalse(TEXT("Cannot equip item not in inventory"), Inv->EquipItem(Pistol));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipNonEquippableFails,
	"FederationGame.Inventory.Component.EquipNonEquippableItemFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryEquipNonEquippableFails::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UItemBase* Junk = MakeItem();
	Inv->AddItem(Junk);
	TestFalse(TEXT("Plain item should not be equippable"), Inv->EquipItem(Junk));
	return true;
}

// ---------------------------------------------------------------------------
// EquipItemToSlot
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipToSlotDirect,
	"FederationGame.Inventory.Component.EquipItemToSlotDirect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryEquipToSlotDirect::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	UWeaponItem* Knife = NewObject<UWeaponItem>();
	Knife->Weight = 1.f;
	Inv->AddItem(Knife);
	TestTrue(TEXT("EquipItemToSlot should succeed"), Inv->EquipItemToSlot(Knife, EEquipmentSlot::SecondaryWeapon));
	TestEqual(TEXT("Secondary should hold knife"),
		Inv->GetEquippedItem(EEquipmentSlot::SecondaryWeapon), static_cast<UItemBase*>(Knife));
	TestNull(TEXT("Primary should be empty"), Inv->GetEquippedItem(EEquipmentSlot::PrimaryWeapon));
	TestFalse(TEXT("Item removed from items"), Inv->HasItem(Knife));
	return true;
}

// ---------------------------------------------------------------------------
// Slot family compatibility
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSlotFamilyWeapons,
	"FederationGame.Inventory.Component.SlotFamilyWeaponsCompatible",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FSlotFamilyWeapons::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Primary-Secondary compatible"),
		UInventoryComponent::AreSlotsFamilyCompatible(EEquipmentSlot::PrimaryWeapon, EEquipmentSlot::SecondaryWeapon));
	TestTrue(TEXT("Secondary-Primary compatible"),
		UInventoryComponent::AreSlotsFamilyCompatible(EEquipmentSlot::SecondaryWeapon, EEquipmentSlot::PrimaryWeapon));
	TestFalse(TEXT("Primary-Head incompatible"),
		UInventoryComponent::AreSlotsFamilyCompatible(EEquipmentSlot::PrimaryWeapon, EEquipmentSlot::Head));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSlotFamilyAbilities,
	"FederationGame.Inventory.Component.SlotFamilyAbilitiesCompatible",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FSlotFamilyAbilities::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Ability1-Ability2 compatible"),
		UInventoryComponent::AreSlotsFamilyCompatible(EEquipmentSlot::Ability1, EEquipmentSlot::Ability2));
	TestFalse(TEXT("Ability1-Biomorph1 incompatible"),
		UInventoryComponent::AreSlotsFamilyCompatible(EEquipmentSlot::Ability1, EEquipmentSlot::Biomorph1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSlotFamilyBiomorphs,
	"FederationGame.Inventory.Component.SlotFamilyBiomorphsCompatible",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FSlotFamilyBiomorphs::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Bio1-Bio2 compatible"),
		UInventoryComponent::AreSlotsFamilyCompatible(EEquipmentSlot::Biomorph1, EEquipmentSlot::Biomorph2));
	TestTrue(TEXT("Bio2-Bio3 compatible"),
		UInventoryComponent::AreSlotsFamilyCompatible(EEquipmentSlot::Biomorph2, EEquipmentSlot::Biomorph3));
	TestTrue(TEXT("Bio1-Bio3 compatible"),
		UInventoryComponent::AreSlotsFamilyCompatible(EEquipmentSlot::Biomorph1, EEquipmentSlot::Biomorph3));
	TestFalse(TEXT("Bio1-Shield incompatible"),
		UInventoryComponent::AreSlotsFamilyCompatible(EEquipmentSlot::Biomorph1, EEquipmentSlot::Shield));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSlotFamilySameSlot,
	"FederationGame.Inventory.Component.SlotFamilySameSlotCompatible",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FSlotFamilySameSlot::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Head-Head compatible"),
		UInventoryComponent::AreSlotsFamilyCompatible(EEquipmentSlot::Head, EEquipmentSlot::Head));
	TestTrue(TEXT("Shoes-Shoes compatible"),
		UInventoryComponent::AreSlotsFamilyCompatible(EEquipmentSlot::Shoes, EEquipmentSlot::Shoes));
	return true;
}

// ---------------------------------------------------------------------------
// Delegate
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryDelegateFires,
	"FederationGame.Inventory.Component.DelegateFiresOnChange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryDelegateFires::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = MakeInventory();
	int32 CallCount = 0;
	Inv->OnNativeInventoryChanged.AddLambda([&CallCount]() { ++CallCount; });

	UItemBase* Item = MakeItem();
	Inv->AddItem(Item);
	TestEqual(TEXT("Delegate should fire on add"), CallCount, 1);

	Inv->RemoveItem(Item);
	TestEqual(TEXT("Delegate should fire on remove"), CallCount, 2);

	return true;
}

// ---------------------------------------------------------------------------
// Starter items (character integration)
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterHasInventoryComponent,
	"FederationGame.Inventory.Character.HasInventoryComponent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FCharacterHasInventoryComponent::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World)
	{
		AddError(TEXT("No world context available for spawning actor"));
		return false;
	}

	AFederationCharacter* Character = World->SpawnActor<AFederationCharacter>();
	if (!Character)
	{
		AddError(TEXT("Failed to spawn AFederationCharacter"));
		return false;
	}

	TestNotNull(TEXT("Character should have an InventoryComponent"), Character->InventoryComp.Get());

	Character->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterStarterItemsPresent,
	"FederationGame.Inventory.Character.StarterItemsPresent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FCharacterStarterItemsPresent::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!World)
	{
		AddError(TEXT("No world context available"));
		return false;
	}

	AFederationCharacter* Character = World->SpawnActor<AFederationCharacter>();
	if (!Character || !Character->InventoryComp)
	{
		AddError(TEXT("Failed to spawn character or missing InventoryComp"));
		return false;
	}

	Character->DispatchBeginPlay();

	TestTrue(TEXT("Inventory should have starter items"), Character->InventoryComp->GetItems().Num() > 0);
	TestTrue(TEXT("Total weight should be positive"), Character->InventoryComp->GetCurrentWeight() > 0.f);

	Character->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
