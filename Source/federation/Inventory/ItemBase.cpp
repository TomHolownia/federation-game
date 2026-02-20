// Copyright Federation Game. All Rights Reserved.

#include "Inventory/ItemBase.h"

FPrimaryAssetId UItemBase::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("Item"), ItemID);
}
