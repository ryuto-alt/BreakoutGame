#include "BreakoutClearWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

TSharedRef<SWidget> UBreakoutClearWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CanvasPanel"));
		WidgetTree->RootWidget = Root;

		GameClearBox = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GameClearBox"));
		GameClearBox->SetText(FText::FromString(TEXT("GameClear!!")));
		FSlateFontInfo Font = GameClearBox->GetFont();
		Font.Size = 110;
		GameClearBox->SetFont(Font);
		GameClearBox->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.9f, 0.2f)));
		GameClearBox->SetShadowOffset(FVector2D(3.0f, 3.0f));
		GameClearBox->SetShadowColorAndOpacity(FLinearColor::Black);

		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(GameClearBox);
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetPosition(FVector2D::ZeroVector);
		PanelSlot->SetAutoSize(true);
	}
	return Super::RebuildWidget();
}

void UBreakoutClearWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!GameClearBox)
	{
		return;
	}
	Elapsed += InDeltaTime;
	float Y = 0.0f;
	if (Elapsed < Duration * NumLoops)
	{
		// 0 → -Height → 0 を Duration 秒かけて NumLoops 回
		const float Phase = FMath::Fmod(Elapsed, Duration) / Duration;
		const float Wave = Phase < 0.5f ? Phase * 2.0f : (1.0f - Phase) * 2.0f;
		Y = -Height * FMath::InterpEaseInOut(0.0f, 1.0f, Wave, 2.0f);
	}
	GameClearBox->SetRenderTranslation(FVector2D(0.0f, Y));
}
