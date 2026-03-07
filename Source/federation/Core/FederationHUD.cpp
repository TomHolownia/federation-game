// Copyright Federation Game. All Rights Reserved.

#include "Core/FederationHUD.h"
#include "Core/FederationGameState.h"
#include "Navigation/WaypointSubsystem.h"
#include "Navigation/WaypointComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"

AFederationHUD::AFederationHUD()
{
	bShowDevDiagnostics = true;
	bShowWaypoints = true;
}

void AFederationHUD::DrawHUD()
{
	Super::DrawHUD();

	if (bShowDevDiagnostics && Canvas)
	{
		float NextY = DevTextStartY;
		DrawDevDiagnostics_Implementation(NextY);
	}

	if (bShowWaypoints && Canvas)
	{
		DrawWaypointIndicators();
	}
}

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

	// Diamond marker icon (drawn as a rotated square with lines)
	const float MarkerSize = 6.f;
	const FLinearColor MarkerColor(0.3f, 0.8f, 1.0f, 1.0f);

	// Draw diamond shape: 4 lines forming a diamond
	DrawLine(CenterX, Y - MarkerSize, CenterX + MarkerSize, Y, MarkerColor);
	DrawLine(CenterX + MarkerSize, Y, CenterX, Y + MarkerSize, MarkerColor);
	DrawLine(CenterX, Y + MarkerSize, CenterX - MarkerSize, Y, MarkerColor);
	DrawLine(CenterX - MarkerSize, Y, CenterX, Y - MarkerSize, MarkerColor);

	Y += MarkerSize + 4.f;

	// Name text (centered)
	const FString NameStr = Name.ToString();
	const FLinearColor NameColor(1.f, 1.f, 1.f, 1.f);
	const float NameScale = 1.0f;

	float NameW = 0.f, NameH = 0.f;
	GetTextSize(NameStr, NameW, NameH, GEngine->GetSmallFont(), NameScale);
	DrawText(NameStr, NameColor, CenterX - NameW * 0.5f, Y, GEngine->GetSmallFont(), NameScale, false);

	Y += NameH + 2.f;

	// Distance text (centered, slightly dimmer)
	const FString DistStr = Distance.ToString();
	const FLinearColor DistColor(0.7f, 0.85f, 1.0f, 0.85f);
	const float DistScale = 0.9f;

	float DistW = 0.f, DistH = 0.f;
	GetTextSize(DistStr, DistW, DistH, GEngine->GetSmallFont(), DistScale);
	DrawText(DistStr, DistColor, CenterX - DistW * 0.5f, Y, GEngine->GetSmallFont(), DistScale, false);
}

void AFederationHUD::DrawDevDiagnostics_Implementation(float& OutNextY)
{
	if (!Canvas) return;

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	const float Speed = Pawn ? Pawn->GetVelocity().Size() : 0.f;

	UWorld* World = GetWorld();
	AFederationGameState* GS = World ? World->GetGameState<AFederationGameState>() : nullptr;
	FString LevelStr = TEXT("?");
	if (GS)
	{
		if (GS->DebugStreamingState == TEXT("Idle")) LevelStr = TEXT("Deep Space");
		else if (GS->DebugStreamingState == TEXT("Loading")) LevelStr = TEXT("Transitioning level");
		else if (GS->DebugStreamingState == TEXT("OnSurface")) LevelStr = TEXT("Planet surface");
		else if (GS->DebugStreamingState == TEXT("Unloading")) LevelStr = TEXT("Leaving surface");
		else LevelStr = GS->DebugStreamingState;
	}
	const FString LevelName = (GS && !GS->DebugStreamingLevelName.IsEmpty()) ? GS->DebugStreamingLevelName : FString();

	const FLinearColor Color(0.9f, 0.95f, 1.f, 1.f);
	const float Scale = 1.2f;

	Canvas->SetDrawColor(FColor::White);
	DrawText(FString::Printf(TEXT("Speed: %.0f"), Speed), Color, 24.f, OutNextY, nullptr, Scale, false);
	OutNextY += DevTextLineHeight;

	DrawText(FString::Printf(TEXT("Level: %s"), *LevelStr), Color, 24.f, OutNextY, nullptr, Scale, false);
	OutNextY += DevTextLineHeight;

	if (!LevelName.IsEmpty())
	{
		DrawText(FString::Printf(TEXT("Stream: %s"), *LevelName), Color, 24.f, OutNextY, nullptr, Scale, false);
		OutNextY += DevTextLineHeight;
	}

	if (GS)
	{
		DrawText(
			FString::Printf(TEXT("Jetpack Enabled: %s"), GS->DebugJetpackEnabled ? TEXT("True") : TEXT("False")),
			Color, 24.f, OutNextY, nullptr, Scale, false);
		OutNextY += DevTextLineHeight;
		DrawText(
			FString::Printf(TEXT("Jetpack Boost: %s"), GS->DebugJetpackBoost ? TEXT("True") : TEXT("False")),
			Color, 24.f, OutNextY, nullptr, Scale, false);
		OutNextY += DevTextLineHeight;
	}
}
