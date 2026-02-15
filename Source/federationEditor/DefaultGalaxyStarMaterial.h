// Copyright Federation Game. All Rights Reserved.

#pragma once

class UMaterialInterface;

/** Returns a material suitable for AGalaxyStarField. Creates and saves one if missing so placement never requires manual steps. */
UMaterialInterface* GetOrCreateDefaultGalaxyStarMaterial();
