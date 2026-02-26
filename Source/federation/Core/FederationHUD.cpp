// Copyright Federation Game. All Rights Reserved.

#include "Core/FederationHUD.h"
#include "Core/FederationGameState.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"

AFederationHUD::AFederationHUD()
{
	bShowDevDiagnostics = true;
}

void AFederationHUD::DrawHUD()
{
	Super::DrawHUD();

	if (bShowDevDiagnostics && Canvas)
	{
		float NextY = DevTextStartY;
		DrawDevDiagnostics_Implementation(NextY);
	}
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
	// Map internal state to dev-friendly labels (Idle = deep space, not "loading" a surface)
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
}