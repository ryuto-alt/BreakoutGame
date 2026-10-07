#include "BreakoutGameOverWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

TSharedRef<SWidget> UBreakoutGameOverWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CanvasPanel"));
		WidgetTree->RootWidget = Root;

		GameOverBox = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GameOverBox"));
		GameOverBox->SetText(FText::FromString(TEXT("GameOver!")));
		FSlateFontInfo Font = GameOverBox->GetFont();
		Font.Size = 100;
		GameOverBox->SetFont(Font);
		GameOverBox->SetShadowOffset(FVector2D(3.0f, 3.0f));
		GameOverBox->SetShadowColorAndOpacity(FLinearColor::Black);

		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(GameOverBox);
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetPosition(FVector2D::ZeroVector);
		PanelSlot->SetAutoSize(true);

		// 「Push SPACE to Restart」は GameOver の下
		PushSpaceTextBox = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PushSpaceTextBox"));
		PushSpaceTextBox->SetText(FText::FromString(TEXT("Push SPACE to Restart")));
		FSlateFontInfo SmallFont = PushSpaceTextBox->GetFont();
		SmallFont.Size = 50;
		PushSpaceTextBox->SetFont(SmallFont);
		PushSpaceTextBox->SetShadowOffset(FVector2D(2.0f, 2.0f));
		PushSpaceTextBox->SetShadowColorAndOpacity(FLinearColor::Black);

		UCanvasPanelSlot* PushSlot = Root->AddChildToCanvas(PushSpaceTextBox);
		PushSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PushSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PushSlot->SetPosition(FVector2D(0.0f, 110.0f));
		PushSlot->SetAutoSize(true);
	}
	return Super::RebuildWidget();
}

void UBreakoutGameOverWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (GameOverBox && Elapsed < ColorDuration + InDeltaTime)
	{
		Elapsed += InDeltaTime;
		// 白 → 赤（ColorAndOpacity）
		const float Alpha = FMath::Clamp(Elapsed / ColorDuration, 0.0f, 1.0f);
		GameOverBox->SetColorAndOpacity(FSlateColor(FMath::Lerp(FLinearColor::White, FLinearColor::Red, Alpha)));
	}
}
