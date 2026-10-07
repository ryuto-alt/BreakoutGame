#include "BreakoutTitleWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

namespace
{
	UTextBlock* AddText(UWidgetTree* Tree, UCanvasPanel* Root, const TCHAR* Name, const TCHAR* Text, int32 Size, const FVector2D& Position, const FLinearColor& Color)
	{
		UTextBlock* Box = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Box->SetText(FText::FromString(Text));
		FSlateFontInfo Font = Box->GetFont();
		Font.Size = Size;
		Box->SetFont(Font);
		Box->SetColorAndOpacity(FSlateColor(Color));
		Box->SetShadowOffset(FVector2D(4.0f, 4.0f));
		Box->SetShadowColorAndOpacity(FLinearColor::Black);

		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Box);
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetPosition(Position);
		PanelSlot->SetAutoSize(true);
		return Box;
	}
}

TSharedRef<SWidget> UBreakoutTitleWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CanvasPanel"));
		WidgetTree->RootWidget = Root;

		TitleTextBox = AddText(WidgetTree, Root, TEXT("TitleTextBox"), TEXT("BREAKOUT"), 120, FVector2D(0.0f, -30.0f), FLinearColor(1.0f, 0.9f, 0.2f));
		PushSpaceTextBox = AddText(WidgetTree, Root, TEXT("PushSpaceTextBox"), TEXT("Push SPACE"), 50, FVector2D(0.0f, 130.0f), FLinearColor::White);
	}
	return Super::RebuildWidget();
}

void UBreakoutTitleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	Elapsed += InDeltaTime;
	if (TitleTextBox)
	{
		// タイトルはゆっくり上下にふわふわ
		TitleTextBox->SetRenderTranslation(FVector2D(0.0f, FMath::Sin(Elapsed * 2.0f) * 10.0f));
	}
	if (PushSpaceTextBox)
	{
		// Push SPACE はなめらかに点滅
		PushSpaceTextBox->SetRenderOpacity(0.5f + 0.5f * FMath::Sin(Elapsed * 5.0f));
	}
}
