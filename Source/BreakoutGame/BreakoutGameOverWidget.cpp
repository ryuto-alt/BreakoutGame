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
	}
	return Super::RebuildWidget();
}
