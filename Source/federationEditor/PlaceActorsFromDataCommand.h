// Copyright Federation Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UToolMenu;

/**
 * Registers the "Place Actors From Data" editor command.
 * Reads JSON files from Config/PlacementData/ and spawns actors in the current level.
 * Tools -> Federation -> Place Actors From Data shows a list of available placement presets (*.json).
 * Enables AI/agent workflow: add or edit JSON in that directory and run a preset to place actors.
 */
class FPlaceActorsFromDataCommand
{
public:
	static void Register();
	static void Unregister();

	/** Run placement from a specific file in Config/PlacementData/ (e.g. "GalaxyMapTest.json"). */
	static void Execute(const FString& RelativeFileName);

private:
	static void RegisterMenus();
	/** Populate the "Place Actors From Data" submenu with one entry per *.json in Config/PlacementData/. */
	static void FillPlaceActorsSubmenu(UToolMenu* Menu);
	static FString GetPlacementDataDir();
	/** Log selected GalaxyStarField's key properties to Output Log (for comparing working vs spawned). */
	static void LogSelectedStarFieldProperties();
	/** Write selected GalaxyStarField's StarMesh/StarMaterial into PlacementData.json Defaults so future Place Actors From Data uses them. */
	static void UseSelectedStarFieldAsPlacementDefault();
};
