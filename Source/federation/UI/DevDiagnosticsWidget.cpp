// Copyright Federation Game. All Rights Reserved.

#include "UI/DevDiagnosticsWidget.h"
#include "Core/FederationGameState.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Spacer.h"
#include "Blueprint/WidgetTree.h"
#include "Styling/CoreStyle.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

namespace DevDiag
{
	static const FSlateColor Accent = FSlateColor(FLinearColor(0.3f, 0.8f, 1.0f));
	static const FSlateColor Content = FSlateColor(FLinearColor(0.9f, 0.95f, 1.f));
}

void UDevDiagnosticsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
}

void UDevDiagnosticsWidget::BuildWidgetTree()
{
	if (!WidgetTree) return;

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Root;

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
	Background->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.05f, 0.8f));
	Background->SetPadding(FMargin(16.f, 12.f));

	UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Background);
	BgSlot->SetAnchors(FAnchors(0.f, 0.f));
	BgSlot->SetPosition(FVector2D(24.f, 24.f));
	BgSlot->SetAutoSize(true);

	UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
	Background->AddChild(VBox);

	FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 14);
	FSlateFontInfo ContentFont = FCoreStyle::GetDefaultFontStyle("Regular", 12);

	// Title
	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
	Title->SetText(FText::FromString(TEXT("DEV DIAGNOSTICS")));
	Title->SetColorAndOpacity(DevDiag::Accent);
	Title->SetFont(TitleFont);
	VBox->AddChildToVerticalBox(Title);

	// Spacer after title
	USpacer* Gap = WidgetTree->ConstructWidget<USpacer>();
	Gap->SetSize(FVector2D(0.f, 6.f));
	VBox->AddChildToVerticalBox(Gap);

	auto MakeLine = [&](TObjectPtr<UTextBlock>& OutBlock, const FString& Default)
	{
		OutBlock = WidgetTree->ConstructWidget<UTextBlock>();
		OutBlock->SetText(FText::FromString(Default));
		OutBlock->SetColorAndOpacity(DevDiag::Content);
		OutBlock->SetFont(ContentFont);
		UVerticalBoxSlot* Slot = VBox->AddChildToVerticalBox(OutBlock);
		Slot->SetPadding(FMargin(0.f, 1.f));
	};

	MakeLine(SpeedText, TEXT("Speed: 0"));
	MakeLine(LevelText, TEXT("Level: ?"));
	MakeLine(StreamText, TEXT("Stream: -"));
	MakeLine(JetpackEnabledText, TEXT("Jetpack: False"));
	MakeLine(JetpackBoostText, TEXT("Boost: False"));
}

void UDevDiagnosticsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	const float Speed = Pawn ? Pawn->GetVelocity().Size() : 0.f;
	if (SpeedText)
	{
		SpeedText->SetText(FText::FromString(FString::Printf(TEXT("Speed: %.0f"), Speed)));
	}

	UWorld* World = GetWorld();
	AFederationGameState* GS = World ? World->GetGameState<AFederationGameState>() : nullptr;
	if (!GS) return;

	if (LevelText)
	{
		FString LevelStr;
		if (GS->DebugStreamingState == TEXT("Idle")) LevelStr = TEXT("Deep Space");
		else if (GS->DebugStreamingState == TEXT("Loading")) LevelStr = TEXT("Transitioning level");
		else if (GS->DebugStreamingState == TEXT("OnSurface")) LevelStr = TEXT("Planet surface");
		else if (GS->DebugStreamingState == TEXT("Unloading")) LevelStr = TEXT("Leaving surface");
		else LevelStr = GS->DebugStreamingState;
		LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level: %s"), *LevelStr)));
	}

	if (StreamText)
	{
		StreamText->SetText(FText::FromString(
			GS->DebugStreamingLevelName.IsEmpty()
				? TEXT("Stream: -")
				: FString::Printf(TEXT("Stream: %s"), *GS->DebugStreamingLevelName)));
	}

	if (JetpackEnabledText)
	{
		JetpackEnabledText->SetText(FText::FromString(
			FString::Printf(TEXT("Jetpack: %s"), GS->DebugJetpackEnabled ? TEXT("True") : TEXT("False"))));
	}

	if (JetpackBoostText)
	{
		JetpackBoostText->SetText(FText::FromString(
			FString::Printf(TEXT("Boost: %s"), GS->DebugJetpackBoost ? TEXT("True") : TEXT("False"))));
	}
}
