// Copyright Federation Game. All Rights Reserved.

#include "Core/FederationHUD.h"
#include "Core/FederationGameState.h"
#include "UI/DevDiagnosticsWidget.h"
#include "UI/InventoryWidget.h"
#include "Inventory/InventoryComponent.h"
#include "Navigation/WaypointSubsystem.h"
#include "Navigation/WaypointComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"

AFederationHUD::AFederationHUD()
{
	bShowWaypoints = true;
}

void AFederationHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	DevDiagnosticsWidget = CreateWidget<UDevDiagnosticsWidget>(PC);
	InventoryWidget = CreateWidget<UInventoryWidget>(PC);
}

// ---------------------------------------------------------------------------
// Widget toggles
// ---------------------------------------------------------------------------

void AFederationHUD::ToggleDevDiagnostics()
{
	if (!DevDiagnosticsWidget) return;

	if (DevDiagnosticsWidget->IsInViewport())
	{
		DevDiagnosticsWidget->RemoveFromParent();
	}
	else
	{
		DevDiagnosticsWidget->AddToViewport(0);
	}
}

void AFederationHUD::ToggleInventory()
{
	if (!InventoryWidget) return;

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	if (InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromParent();
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
	else
	{
		if (!InventoryWidget->HasInventoryComponent())
		{
			APawn* Pawn = PC->GetPawn();
			if (Pawn)
			{
				UInventoryComponent* Inv = Pawn->FindComponentByClass<UInventoryComponent>();
				if (Inv)
				{
					InventoryWidget->SetInventoryComponent(Inv);
				}
			}
		}

		InventoryWidget->AddToViewport(10);
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI().SetWidgetToFocus(InventoryWidget->TakeWidget()));
	}
}

bool AFederationHUD::IsInventoryOpen() const
{
	return InventoryWidget && InventoryWidget->IsInViewport();
}

// ---------------------------------------------------------------------------
// DrawHUD — waypoint indicators only
// ---------------------------------------------------------------------------

void AFederationHUD::DrawHUD()
{
	Super::DrawHUD();

	if (bShowWaypoints && Canvas)
	{
		DrawWaypointIndicators();
	}
}

// ---------------------------------------------------------------------------
// Waypoint canvas drawing (unchanged from original)
// ---------------------------------------------------------------------------

void AFederationHUD::DrawWaypointIndicators()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UWaypointSubsystem* Sub = World->GetSubsystem<UWaypointSubsystem>();
	if (!Sub) return;

	const FVector PlayerLoc = Pawn->GetActorLocation();
	TArray<UWaypointComponent*> ActiveWaypoints = Sub->GetAllActiveWaypoints();

	for (UWaypointComponent* WP : ActiveWaypoints)
	{
		const FVector WPLoc = WP->GetWaypointLocation();
		const double Dist = FVector::Dist(WPLoc, PlayerLoc);

		if (WP->MaxVisibleDistance > 0.0 && Dist > WP->MaxVisibleDistance)
		{
			continue;
		}

		FVector2D ScreenPos;
		const bool bOnScreen = PC->ProjectWorldLocationToScreen(WPLoc, ScreenPos, true);
		if (!bOnScreen)
		{
			continue;
		}

		const FText DistText = UWaypointComponent::FormatDistance(Dist);
		DrawWaypointMarker(ScreenPos, WP->DisplayName, DistText);
	}
}

void AFederationHUD::DrawWaypointMarker(const FVector2D& ScreenPos, const FText& Name, const FText& Distance)
{
	if (!Canvas) return;

	const float CenterX = ScreenPos.X;
	float Y = ScreenPos.Y;

	const float MarkerSize = 6.f;
	const FLinearColor MarkerColor(0.3f, 0.8f, 1.0f, 1.0f);

	DrawLine(CenterX, Y - MarkerSize, CenterX + MarkerSize, Y, MarkerColor);
	DrawLine(CenterX + MarkerSize, Y, CenterX, Y + MarkerSize, MarkerColor);
	DrawLine(CenterX, Y + MarkerSize, CenterX - MarkerSize, Y, MarkerColor);
	DrawLine(CenterX - MarkerSize, Y, CenterX, Y - MarkerSize, MarkerColor);

	Y += MarkerSize + 4.f;

	const FString NameStr = Name.ToString();
	const FLinearColor NameColor(1.f, 1.f, 1.f, 1.f);
	const float NameScale = 1.0f;

	float NameW = 0.f, NameH = 0.f;
	GetTextSize(NameStr, NameW, NameH, GEngine->GetSmallFont(), NameScale);
	DrawText(NameStr, NameColor, CenterX - NameW * 0.5f, Y, GEngine->GetSmallFont(), NameScale, false);

	Y += NameH + 2.f;

	const FString DistStr = Distance.ToString();
	const FLinearColor DistColor(0.7f, 0.85f, 1.0f, 0.85f);
	const float DistScale = 0.9f;

	float DistW = 0.f, DistH = 0.f;
	GetTextSize(DistStr, DistW, DistH, GEngine->GetSmallFont(), DistScale);
	DrawText(DistStr, DistColor, CenterX - DistW * 0.5f, Y, GEngine->GetSmallFont(), DistScale, false);
}
