// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UI/InventoryWidget.h"
#include "UI/ItemTileWidget.h"
#include "UI/EquipmentSlotWidget.h"
#include "UI/ItemDragDropOperation.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemBase.h"
#include "Inventory/WeaponItem.h"
#include "Inventory/EquipmentItem.h"
#include "Inventory/ItemTypes.h"
#include "Character/FederationCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Widget classes exist
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryWidgetExists,
	"FederationGame.UI.InventoryWidget.ClassExists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FInventoryWidgetExists::RunTest(const FString& Parameters)
{
	UClass* WidgetClass = UInventoryWidget::StaticClass();
	TestNotNull("InventoryWidget class exists", WidgetClass);
	TestTrue("Is a UUserWidget subclass", WidgetClass->IsChildOf(UUserWidget::StaticClass()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemTileWidgetExists,
	"FederationGame.UI.ItemTileWidget.ClassExists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FItemTileWidgetExists::RunTest(const FString& Parameters)
{
	UClass* WidgetClass = UItemTileWidget::StaticClass();
	TestNotNull("ItemTileWidget class exists", WidgetClass);
	TestTrue("Is a UUserWidget subclass", WidgetClass->IsChildOf(UUserWidget::StaticClass()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEquipmentSlotWidgetExists,
	"FederationGame.UI.EquipmentSlotWidget.ClassExists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FEquipmentSlotWidgetExists::RunTest(const FString& Parameters)
{
	UClass* WidgetClass = UEquipmentSlotWidget::StaticClass();
	TestNotNull("EquipmentSlotWidget class exists", WidgetClass);
	TestTrue("Is a UUserWidget subclass", WidgetClass->IsChildOf(UUserWidget::StaticClass()));
	return true;
}

// ---------------------------------------------------------------------------
// Drag drop operation
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDragDropOpExists,
	"FederationGame.UI.ItemDragDropOperation.ClassExists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDragDropOpExists::RunTest(const FString& Parameters)
{
	UClass* OpClass = UItemDragDropOperation::StaticClass();
	TestNotNull("ItemDragDropOperation class exists", OpClass);
	TestTrue("Is a UDragDropOperation subclass", OpClass->IsChildOf(UDragDropOperation::StaticClass()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDragDropOpHoldsItem,
	"FederationGame.UI.ItemDragDropOperation.HoldsItem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDragDropOpHoldsItem::RunTest(const FString& Parameters)
{
	UItemDragDropOperation* Op = NewObject<UItemDragDropOperation>();
	TestNull("DraggedItem is null by default", Op->DraggedItem.Get());

	UItemBase* Item = NewObject<UItemBase>();
	Item->ItemID = FName("TestItem");
	Op->DraggedItem = Item;
	TestEqual("DraggedItem is set", Op->DraggedItem.Get(), Item);
	return true;
}

// ---------------------------------------------------------------------------
// Slot display names
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSlotNames,
	"FederationGame.UI.InventoryWidget.SlotDisplayNames",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FSlotNames::RunTest(const FString& Parameters)
{
	TestEqual("Head", UInventoryWidget::GetSlotDisplayName(EEquipmentSlot::Head).ToString(), FString("Head"));
	TestEqual("Body", UInventoryWidget::GetSlotDisplayName(EEquipmentSlot::Body).ToString(), FString("Body"));
	TestEqual("Primary", UInventoryWidget::GetSlotDisplayName(EEquipmentSlot::PrimaryWeapon).ToString(), FString("Primary"));
	TestEqual("Secondary", UInventoryWidget::GetSlotDisplayName(EEquipmentSlot::SecondaryWeapon).ToString(), FString("Secondary"));
	TestEqual("Shield", UInventoryWidget::GetSlotDisplayName(EEquipmentSlot::Shield).ToString(), FString("Shield"));
	TestEqual("Ability 1", UInventoryWidget::GetSlotDisplayName(EEquipmentSlot::Ability1).ToString(), FString("Ability 1"));
	TestEqual("Ability 2", UInventoryWidget::GetSlotDisplayName(EEquipmentSlot::Ability2).ToString(), FString("Ability 2"));
	TestEqual("Bio 1", UInventoryWidget::GetSlotDisplayName(EEquipmentSlot::Biomorph1).ToString(), FString("Bio 1"));
	TestEqual("Bio 2", UInventoryWidget::GetSlotDisplayName(EEquipmentSlot::Biomorph2).ToString(), FString("Bio 2"));
	TestEqual("Bio 3", UInventoryWidget::GetSlotDisplayName(EEquipmentSlot::Biomorph3).ToString(), FString("Bio 3"));
	return true;
}

// ---------------------------------------------------------------------------
// Component binding
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWidgetHasNoInventoryByDefault,
	"FederationGame.UI.InventoryWidget.NoInventoryByDefault",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWidgetHasNoInventoryByDefault::RunTest(const FString& Parameters)
{
	UInventoryWidget* Widget = NewObject<UInventoryWidget>();
	TestNotNull("Widget created", Widget);
	TestFalse("No inventory component by default", Widget->HasInventoryComponent());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWidgetBindsInventory,
	"FederationGame.UI.InventoryWidget.BindsInventoryComponent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWidgetBindsInventory::RunTest(const FString& Parameters)
{
	UInventoryWidget* Widget = NewObject<UInventoryWidget>();
	UInventoryComponent* Inv = NewObject<UInventoryComponent>();
	Widget->SetInventoryComponent(Inv);
	TestTrue("Has inventory after binding", Widget->HasInventoryComponent());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWidgetClearsInventory,
	"FederationGame.UI.InventoryWidget.ClearsInventoryOnNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FWidgetClearsInventory::RunTest(const FString& Parameters)
{
	UInventoryWidget* Widget = NewObject<UInventoryWidget>();
	UInventoryComponent* Inv = NewObject<UInventoryComponent>();
	Widget->SetInventoryComponent(Inv);
	TestTrue("Has inventory", Widget->HasInventoryComponent());

	Widget->SetInventoryComponent(nullptr);
	TestFalse("No inventory after clearing", Widget->HasInventoryComponent());
	return true;
}

// ---------------------------------------------------------------------------
// Character input actions
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterHasDevHUDAction,
	"FederationGame.UI.Character.HasToggleDevHUDAction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FCharacterHasDevHUDAction::RunTest(const FString& Parameters)
{
	FProperty* Prop = AFederationCharacter::StaticClass()->FindPropertyByName(FName("ToggleDevHUDAction"));
	TestNotNull("ToggleDevHUDAction property exists", Prop);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterHasInventoryAction,
	"FederationGame.UI.Character.HasToggleInventoryAction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FCharacterHasInventoryAction::RunTest(const FString& Parameters)
{
	FProperty* Prop = AFederationCharacter::StaticClass()->FindPropertyByName(FName("ToggleInventoryAction"));
	TestNotNull("ToggleInventoryAction property exists", Prop);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
