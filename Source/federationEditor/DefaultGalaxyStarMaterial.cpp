// Copyright Federation Game. All Rights Reserved.

#include "DefaultGalaxyStarMaterial.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "MaterialEditingLibrary.h"
#include "Editor.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogDefaultGalaxyStarMaterial, Log, All);

namespace
{
	static const TCHAR* DefaultGalaxyStarMaterialPath = TEXT("/Game/Federation/Materials/M_GalaxyStar.M_GalaxyStar");
}

UMaterialInterface* GetOrCreateDefaultGalaxyStarMaterial()
{
	UMaterialInterface* Loaded = LoadObject<UMaterialInterface>(nullptr, DefaultGalaxyStarMaterialPath);
	if (Loaded)
	{
		return Loaded;
	}

	FString PackageName = TEXT("/Game/Federation/Materials/M_GalaxyStar");
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		UE_LOG(LogDefaultGalaxyStarMaterial, Warning, TEXT("CreatePackage failed for %s"), DefaultGalaxyStarMaterialPath);
		return LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
	}

	UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
	UMaterial* Material = Cast<UMaterial>(Factory->FactoryCreateNew(
		UMaterial::StaticClass(), Package, FName(TEXT("M_GalaxyStar")),
		RF_Public | RF_Standalone, nullptr, GWarn));
	if (!Material)
	{
		UE_LOG(LogDefaultGalaxyStarMaterial, Warning, TEXT("FactoryCreateNew failed for M_GalaxyStar"));
		return LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
	}

	// Add emissive constant (warm white) so the star field mesh is visible with zero manual steps
	UMaterialExpression* Expr = UMaterialEditingLibrary::CreateMaterialExpressionEx(
		Material, nullptr, UMaterialExpressionConstant3Vector::StaticClass(), nullptr, -300, 0, true);
	UMaterialExpressionConstant3Vector* EmissiveExpr = Cast<UMaterialExpressionConstant3Vector>(Expr);
	if (EmissiveExpr)
	{
		// Bright warm white so stars are clearly visible (values > 1 for HDR emissive)
		EmissiveExpr->Constant = FLinearColor(4.0f, 3.8f, 3.6f);
		UMaterialEditingLibrary::ConnectMaterialProperty(EmissiveExpr, FString(), MP_EmissiveColor);
	}
	UMaterialEditingLibrary::RecompileMaterial(Material);
	Material->PreEditChange(nullptr);
	Material->PostEditChange();
	Material->MarkPackageDirty();

	FAssetRegistryModule::AssetCreated(Material);

	FString Filename;
	if (FPackageName::TryConvertLongPackageNameToFilename(PackageName, Filename, FPackageName::GetAssetPackageExtension()))
	{
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		UPackage::SavePackage(Package, Material, *Filename, SaveArgs);
	}

	return Material;
}
