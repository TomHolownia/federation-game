// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Registers the "Place Actors From Data" editor command.
 * Reads Config/PlacementData.json and spawns actors in the current level.
 * Enables AI/agent workflow: edit JSON and run this to place actors without clicking in the editor.
 */
class FPlaceActorsFromDataCommand
{
public:
	static void Register();
	static void Unregister();

private:
	static void Execute();
	static void RegisterMenus();
};
