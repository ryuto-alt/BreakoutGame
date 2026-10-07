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
		GameOverBox->SetShadowOffset(FVector2D(4.0f, 4.0f));
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
		PushSpaceTextBox->SetShadowOffset(FVector2D(3.0f, 3.0f));
		PushSpaceTextBox->SetShadowColorAndOpacity(FLinearColor::Black);
		PushSpaceTextBox->SetRenderOpacity(0.0f);

		UCanvasPanelSlot* PushSlot = Root->AddChildToCanvas(PushSpaceTextBox);
		PushSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PushSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PushSlot->SetPosition(FVector2D(0.0f, 120.0f));
		PushSlot->SetAutoSize(true);
	}
	return Super::RebuildWidget();
}

void UBreakoutGameOverWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	Elapsed += InDeltaTime;
	if (GameOverBox)
	{
		// 白 → 赤（ColorAndOpacity）
		const float Alpha = FMath::Clamp(Elapsed / ColorDuration, 0.0f, 1.0f);
		GameOverBox->SetColorAndOpacity(FSlateColor(FMath::Lerp(FLinearColor::White, FLinearColor::Red, Alpha)));

		// 大きく出てきて、少しオーバーシュートしながら縮む（ポップ）
		const float PopTime = 0.45f;
		const float P = FMath::Clamp(Elapsed / PopTime, 0.0f, 1.0f);
		const float Scale = 1.0f + 2.2f * FMath::Pow(1.0f - P, 2.0f) - 0.12f * FMath::Sin(P * PI);
		GameOverBox->SetRenderScale(FVector2D(Scale, Scale));

		// 最初の0.7秒は左右に揺れる
		const float ShakeTime = 0.7f;
		const float Shake = Elapsed < ShakeTime ? FMath::Sin(Elapsed * 90.0f) * 14.0f * (1.0f - Elapsed / ShakeTime) : 0.0f;
		GameOverBox->SetRenderTranslation(FVector2D(Shake, 0.0f));
	}
	if (PushSpaceTextBox)
	{
		// 文字が落ち着いたあとで点滅
		const float Start = 0.6f;
		PushSpaceTextBox->SetRenderOpacity(Elapsed < Start ? 0.0f : 0.55f + 0.45f * FMath::Sin((Elapsed - Start) * 6.0f));
	}
}
