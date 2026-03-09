// Copyright Federation Game. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UI/DevDiagnosticsWidget.h"
#include "Core/FederationHUD.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// Widget creation
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDevDiagCreates,
	"FederationGame.UI.DevDiagnostics.WidgetClassExists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FDevDiagCreates::RunTest(const FString& Parameters)
{
	UClass* WidgetClass = UDevDiagnosticsWidget::StaticClass();
	TestNotNull("DevDiagnosticsWidget class exists", WidgetClass);
	TestTrue("Is a UUserWidget subclass", WidgetClass->IsChildOf(UUserWidget::StaticClass()));
	return true;
}

// ---------------------------------------------------------------------------
// HUD has widget management methods
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDHasToggleDev,
	"FederationGame.UI.HUD.HasToggleDevDiagnostics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FHUDHasToggleDev::RunTest(const FString& Parameters)
{
	UFunction* Func = AFederationHUD::StaticClass()->FindFunctionByName(FName("ToggleDevDiagnostics"));
	TestNotNull("ToggleDevDiagnostics function exists", Func);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDHasToggleInv,
	"FederationGame.UI.HUD.HasToggleInventory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FHUDHasToggleInv::RunTest(const FString& Parameters)
{
	UFunction* Func = AFederationHUD::StaticClass()->FindFunctionByName(FName("ToggleInventory"));
	TestNotNull("ToggleInventory function exists", Func);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHUDIsInventoryOpen,
	"FederationGame.UI.HUD.IsInventoryOpenDefaultFalse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FHUDIsInventoryOpen::RunTest(const FString& Parameters)
{
	AFederationHUD* HUD = NewObject<AFederationHUD>();
	TestNotNull("HUD created", HUD);
	TestFalse("Inventory not open by default", HUD->IsInventoryOpen());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
