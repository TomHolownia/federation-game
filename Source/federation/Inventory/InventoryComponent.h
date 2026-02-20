// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/ItemTypes.h"
#include "InventoryComponent.generated.h"

class UItemBase;
class UEquipmentItem;
class UWeaponItem;

USTRUCT(BlueprintType)
struct FInventoryEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UItemBase> ItemDef = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Count = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

/**
 * Manages carried items and equipped gear.
 * Attach to any actor that needs an inventory (player characters, NPCs, containers).
 *
 * Quest items bypass weight limits and are stored alongside regular items
 * (filtered by bIsQuestItem on UItemBase).
 */
UCLASS(ClassGroup = "Federation", meta = (BlueprintSpawnableComponent))
class FEDERATION_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// --- Configuration ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	float MaxCarryWeight = 100.f;

	// --- Public API ---

	/** Add Count copies of an item. Returns true if all were added (weight permitting). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UItemBase* Item, int32 Count = 1);

	/** Remove Count copies. Returns true if the inventory had enough to remove. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(UItemBase* Item, int32 Count = 1);

	/** Check whether the inventory contains at least Count of this item. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool HasItem(const UItemBase* Item, int32 Count = 1) const;

	/** Get the count of a specific item in the inventory. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetItemCount(const UItemBase* Item) const;

	/**
	 * Equip an item that is already in the inventory.
	 * Weapons go to PrimaryWeapon (or SecondaryWeapon if primary is occupied).
	 * Equipment items use their Slot property.
	 * Returns true on success.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool EquipItem(UItemBase* Item);

	/** Unequip whatever is in the given slot. Returns the item, or nullptr if the slot was empty. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItemBase* UnequipSlot(EEquipmentSlot Slot);

	/** Get the item currently in a slot, or nullptr. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UItemBase* GetEquippedItem(EEquipmentSlot Slot) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const TArray<FInventoryEntry>& GetItems() const { return Items; }

	// --- Delegate ---

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	// --- State (public for testing) ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryEntry> Items;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TMap<EEquipmentSlot, TObjectPtr<UItemBase>> EquippedItems;

private:
	/** Find the index of an existing entry for this item, or INDEX_NONE. */
	int32 FindEntryIndex(const UItemBase* Item) const;

	/** Resolve which slot a given item should occupy. Returns false if the item is not equippable. */
	bool ResolveSlot(const UItemBase* Item, EEquipmentSlot& OutSlot) const;
};
