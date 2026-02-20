// Copyright Federation Game. All Rights Reserved.

#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemBase.h"
#include "Inventory/EquipmentItem.h"
#include "Inventory/WeaponItem.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInventoryComponent::AddItem(UItemBase* Item, int32 Count)
{
	if (!Item || Count <= 0)
	{
		return false;
	}

	if (!Item->bIsQuestItem)
	{
		const float AdditionalWeight = Item->Weight * Count;
		if (GetCurrentWeight() + AdditionalWeight > MaxCarryWeight)
		{
			return false;
		}
	}

	const int32 Idx = FindEntryIndex(Item);
	if (Idx != INDEX_NONE)
	{
		Items[Idx].Count += Count;
	}
	else
	{
		FInventoryEntry Entry;
		Entry.ItemDef = Item;
		Entry.Count = Count;
		Items.Add(Entry);
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveItem(UItemBase* Item, int32 Count)
{
	if (!Item || Count <= 0)
	{
		return false;
	}

	const int32 Idx = FindEntryIndex(Item);
	if (Idx == INDEX_NONE)
	{
		return false;
	}

	if (Items[Idx].Count < Count)
	{
		return false;
	}

	Items[Idx].Count -= Count;
	if (Items[Idx].Count <= 0)
	{
		Items.RemoveAt(Idx);
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::HasItem(const UItemBase* Item, int32 Count) const
{
	if (!Item || Count <= 0)
	{
		return false;
	}

	const int32 Idx = FindEntryIndex(Item);
	return Idx != INDEX_NONE && Items[Idx].Count >= Count;
}

int32 UInventoryComponent::GetItemCount(const UItemBase* Item) const
{
	if (!Item)
	{
		return 0;
	}

	const int32 Idx = FindEntryIndex(Item);
	return Idx != INDEX_NONE ? Items[Idx].Count : 0;
}

bool UInventoryComponent::EquipItem(UItemBase* Item)
{
	if (!Item)
	{
		return false;
	}

	if (!HasItem(Item))
	{
		return false;
	}

	EEquipmentSlot TargetSlot;
	if (!ResolveSlot(Item, TargetSlot))
	{
		return false;
	}

	EquippedItems.Add(TargetSlot, Item);
	OnInventoryChanged.Broadcast();
	return true;
}

UItemBase* UInventoryComponent::UnequipSlot(EEquipmentSlot Slot)
{
	TObjectPtr<UItemBase>* Found = EquippedItems.Find(Slot);
	if (!Found || !(*Found))
	{
		return nullptr;
	}

	UItemBase* Removed = *Found;
	EquippedItems.Remove(Slot);
	OnInventoryChanged.Broadcast();
	return Removed;
}

UItemBase* UInventoryComponent::GetEquippedItem(EEquipmentSlot Slot) const
{
	const TObjectPtr<UItemBase>* Found = EquippedItems.Find(Slot);
	return Found ? Found->Get() : nullptr;
}

float UInventoryComponent::GetCurrentWeight() const
{
	float Total = 0.f;
	for (const FInventoryEntry& Entry : Items)
	{
		if (Entry.ItemDef && !Entry.ItemDef->bIsQuestItem)
		{
			Total += Entry.ItemDef->Weight * Entry.Count;
		}
	}
	return Total;
}

int32 UInventoryComponent::FindEntryIndex(const UItemBase* Item) const
{
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].ItemDef == Item)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

bool UInventoryComponent::ResolveSlot(const UItemBase* Item, EEquipmentSlot& OutSlot) const
{
	if (const UWeaponItem* Weapon = Cast<UWeaponItem>(Item))
	{
		if (!EquippedItems.Contains(EEquipmentSlot::PrimaryWeapon) ||
			EquippedItems[EEquipmentSlot::PrimaryWeapon] == nullptr)
		{
			OutSlot = EEquipmentSlot::PrimaryWeapon;
		}
		else
		{
			OutSlot = EEquipmentSlot::SecondaryWeapon;
		}
		return true;
	}

	if (const UEquipmentItem* Equipment = Cast<UEquipmentItem>(Item))
	{
		OutSlot = Equipment->Slot;
		return true;
	}

	return false;
}
