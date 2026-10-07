#include "BreakoutGameInfoWidget.h"

#include "BreakoutGameManager.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	UTextBlock* AddInfoText(UWidgetTree* Tree, UCanvasPanel* Root, const TCHAR* Name, const TCHAR* Text, int32 Size, const FAnchors& Anchors, const FVector2D& Alignment, const FVector2D& Position, const FLinearColor& Color)
	{
		UTextBlock* Box = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Box->SetText(FText::FromString(Text));
		FSlateFontInfo Font = Box->GetFont();
		Font.Size = Size;
		Box->SetFont(Font);
		Box->SetColorAndOpacity(FSlateColor(Color));
		Box->SetShadowOffset(FVector2D(2.0f, 2.0f));
		Box->SetShadowColorAndOpacity(FLinearColor::Black);

		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Box);
		PanelSlot->SetAnchors(Anchors);
		PanelSlot->SetAlignment(Alignment);
		PanelSlot->SetPosition(Position);
		PanelSlot->SetAutoSize(true);
		return Box;
	}
}

TSharedRef<SWidget> UBreakoutGameInfoWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CanvasPanel"));
		WidgetTree->RootWidget = Root;

		// 左上 / 上中央 / 右上
		LeftBallTextBox = AddInfoText(WidgetTree, Root, TEXT("LeftBallTextBox"), TEXT("LeftBall : 0"), 34, FAnchors(0.0f, 0.0f), FVector2D(0.0f, 0.0f), FVector2D(20.0f, 14.0f), FLinearColor::White);
		StageTextBox = AddInfoText(WidgetTree, Root, TEXT("StageTextBox"), TEXT("STAGE 1"), 34, FAnchors(0.5f, 0.0f), FVector2D(0.5f, 0.0f), FVector2D(0.0f, 14.0f), FLinearColor(1.0f, 0.9f, 0.3f));
		ScoreTextBox = AddInfoText(WidgetTree, Root, TEXT("ScoreTextBox"), TEXT("SCORE 0"), 34, FAnchors(1.0f, 0.0f), FVector2D(1.0f, 0.0f), FVector2D(-20.0f, 14.0f), FLinearColor::White);
	}
	return Super::RebuildWidget();
}

void UBreakoutGameInfoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// スライドの「Text を関数バインド」と同じ：毎フレーム読んで表示する
	if (!GameManager.IsValid())
	{
		GameManager = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass()));
	}
	if (GameManager.IsValid())
	{
		if (LeftBallTextBox)
		{
			LeftBallTextBox->SetText(FText::FromString(FString::Printf(TEXT("LeftBall : %d"), GameManager->GetLeftBallNum())));
		}
		if (StageTextBox)
		{
			StageTextBox->SetText(FText::FromString(GameManager->StageName));
		}
		if (ScoreTextBox)
		{
			ScoreTextBox->SetText(FText::FromString(FString::Printf(TEXT("SCORE %06d"), GameManager->Score)));
		}
	}
}
