#include "BreakoutClearWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

namespace
{
	UTextBlock* AddClearText(UWidgetTree* Tree, UCanvasPanel* Root, const TCHAR* Name, const TCHAR* Text, int32 Size, const FVector2D& Position, const FLinearColor& Color)
	{
		UTextBlock* Box = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Box->SetText(FText::FromString(Text));
		FSlateFontInfo Font = Box->GetFont();
		Font.Size = Size;
		Box->SetFont(Font);
		Box->SetColorAndOpacity(FSlateColor(Color));
		Box->SetShadowOffset(FVector2D(3.0f, 3.0f));
		Box->SetShadowColorAndOpacity(FLinearColor::Black);

		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Box);
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetPosition(Position);
		PanelSlot->SetAutoSize(true);
		return Box;
	}
}

TSharedRef<SWidget> UBreakoutClearWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CanvasPanel"));
		WidgetTree->RootWidget = Root;

		GameClearBox = AddClearText(WidgetTree, Root, TEXT("GameClearBox"), TEXT("GameClear!!"), 110, FVector2D::ZeroVector, FLinearColor(1.0f, 0.9f, 0.2f));
		AllClearBox = AddClearText(WidgetTree, Root, TEXT("AllClearBox"), TEXT("ALL CLEAR!!"), 56, FVector2D(0.0f, 100.0f), FLinearColor(0.4f, 1.0f, 0.9f));
		AllClearBox->SetVisibility(ESlateVisibility::Collapsed);
		PushSpaceTextBox = AddClearText(WidgetTree, Root, TEXT("PushSpaceTextBox"), TEXT("Push SPACE to Next Stage"), 40, FVector2D(0.0f, 175.0f), FLinearColor::White);
		PushSpaceTextBox->SetRenderOpacity(0.0f);
	}
	return Super::RebuildWidget();
}

void UBreakoutClearWidget::SetLastStage(bool bInLast)
{
	bLastStage = bInLast;
	if (AllClearBox && PushSpaceTextBox)
	{
		AllClearBox->SetVisibility(bLastStage ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		PushSpaceTextBox->SetText(FText::FromString(bLastStage ? TEXT("Push SPACE to Title") : TEXT("Push SPACE to Next Stage")));
	}
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

	// 最初にぐっと大きく出てから、弾みながら落ち着く（ズームバウンス）
	const float Zoom = FMath::Clamp(Elapsed / 0.55f, 0.0f, 1.0f);
	const float Bounce = 1.0f + 0.9f * FMath::Pow(1.0f - Zoom, 2.0f) * FMath::Cos(Zoom * PI * 2.5f);
	const float Scale = Elapsed < 0.55f ? FMath::Max(Bounce * Zoom + 0.0f, 0.05f) : 1.0f;
	GameClearBox->SetRenderScale(FVector2D(Scale, Scale));

	// 次へ進む案内は少し遅れて点滅
	if (PushSpaceTextBox)
	{
		const float Start = 0.8f;
		PushSpaceTextBox->SetRenderOpacity(Elapsed < Start ? 0.0f : 0.55f + 0.45f * FMath::Sin((Elapsed - Start) * 6.0f));
	}
}
