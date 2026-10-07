#include "BreakoutGameInfoWidget.h"

#include "BreakoutGameManager.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

TSharedRef<SWidget> UBreakoutGameInfoWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CanvasPanel"));
		WidgetTree->RootWidget = Root;

		LeftBallTextBox = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LeftBallTextBox"));
		LeftBallTextBox->SetText(FText::FromString(TEXT("LeftBall : 0")));
		FSlateFontInfo Font = LeftBallTextBox->GetFont();
		Font.Size = 36;
		LeftBallTextBox->SetFont(Font);
		LeftBallTextBox->SetShadowOffset(FVector2D(2.0f, 2.0f));
		LeftBallTextBox->SetShadowColorAndOpacity(FLinearColor::Black);

		// 左上
		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(LeftBallTextBox);
		PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		PanelSlot->SetAlignment(FVector2D(0.0f, 0.0f));
		PanelSlot->SetPosition(FVector2D(20.0f, 10.0f));
		PanelSlot->SetAutoSize(true);
	}
	return Super::RebuildWidget();
}

void UBreakoutGameInfoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// スライドの「Text を関数バインド」と同じ：毎フレーム GetLeftBallNum を読んで表示する
	if (!GameManager.IsValid())
	{
		GameManager = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass()));
	}
	if (GameManager.IsValid() && LeftBallTextBox)
	{
		LeftBallTextBox->SetText(FText::FromString(FString::Printf(TEXT("LeftBall : %d"), GameManager->GetLeftBallNum())));
	}
}
