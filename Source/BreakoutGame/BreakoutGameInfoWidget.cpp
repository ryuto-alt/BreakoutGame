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
		ComboTextBox = AddInfoText(WidgetTree, Root, TEXT("ComboTextBox"), TEXT("COMBO"), 64, FAnchors(0.84f, 0.5f), FVector2D(0.5f, 0.5f), FVector2D(0.0f, -60.0f), FLinearColor(1.0f, 0.85f, 0.2f));
		ComboTextBox->SetJustification(ETextJustify::Center);
		ComboTextBox->SetVisibility(ESlateVisibility::Collapsed);
		// FEVER!! は画面上部の中央に大きく（黒い縁取りと影つき）
		FeverTextBox = AddInfoText(WidgetTree, Root, TEXT("FeverTextBox"), TEXT("FEVER!!"), 96, FAnchors(0.5f, 0.0f), FVector2D(0.5f, 0.0f), FVector2D(0.0f, 90.0f), FLinearColor::White);
		FeverTextBox->SetShadowOffset(FVector2D(5.0f, 5.0f));
		FeverTextBox->SetVisibility(ESlateVisibility::Collapsed);
		{
			FSlateFontInfo FeverFont = FeverTextBox->GetFont();
			FeverFont.OutlineSettings.OutlineSize = 4;
			FeverFont.OutlineSettings.OutlineColor = FLinearColor(0.0f, 0.0f, 0.05f);
			FeverTextBox->SetFont(FeverFont);
		}
		IntroTextBox = AddInfoText(WidgetTree, Root, TEXT("IntroTextBox"), TEXT("STAGE 1"), 120, FAnchors(0.5f, 0.5f), FVector2D(0.5f, 0.5f), FVector2D(0.0f, -80.0f), FLinearColor::White);
		IntroTextBox->SetVisibility(ESlateVisibility::Collapsed);
		ScoreTextBox = AddInfoText(WidgetTree, Root, TEXT("ScoreTextBox"), TEXT("SCORE 0"), 34, FAnchors(1.0f, 0.0f), FVector2D(1.0f, 0.0f), FVector2D(-20.0f, 14.0f), FLinearColor::White);
	}
	return Super::RebuildWidget();
}

void UBreakoutGameInfoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Elapsed += InDeltaTime;

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
			const int32 Multiplier = 1 + GameManager->Combo / 10;
			ScoreTextBox->SetText(FText::FromString(Multiplier > 1 ? FString::Printf(TEXT("SCORE %06d  x%d"), GameManager->Score, Multiplier) : FString::Printf(TEXT("SCORE %06d"), GameManager->Score)));
		}

		// COMBO：増えるたびにぽんと大きくなる
		if (ComboTextBox)
		{
			const int32 Combo = GameManager->Combo;
			if (Combo >= 3 && !GameManager->bIsCleared && !GameManager->bIsGameOver)
			{
				ComboTextBox->SetVisibility(ESlateVisibility::HitTestInvisible);
				ComboTextBox->SetText(FText::FromString(FString::Printf(TEXT("COMBO\nx%d"), Combo)));
				// コンボが多いほど大きく、増えた瞬間にぽんと弾む
				const float Grow = 1.0f + 0.9f * FMath::Clamp(Combo / 60.0f, 0.0f, 1.0f);
				const float Pop = Grow * (1.0f + 0.7f * FMath::Exp(-GameManager->ComboPopAge * 9.0f));
				ComboTextBox->SetRenderScale(FVector2D(Pop, Pop));
				FLinearColor ComboColor = FMath::Lerp(FLinearColor(1.0f, 0.85f, 0.2f), FLinearColor(1.0f, 0.3f, 0.2f), FMath::Clamp(Combo / 40.0f, 0.0f, 1.0f));
				// 途切れるまでの残り時間に合わせて薄くなる
				const float TimeRatio = FMath::Clamp(GameManager->ComboTimeLeft / FMath::Max(GameManager->ComboTimeout, 0.1f), 0.0f, 1.0f);
				ComboColor.A = 0.25f + 0.75f * TimeRatio;
				ComboTextBox->SetColorAndOpacity(FSlateColor(ComboColor));
			}
			else
			{
				ComboTextBox->SetVisibility(ESlateVisibility::Collapsed);
			}
		}

		// FEVER：虹色に点滅しながら揺れる
		if (FeverTextBox)
		{
			if (GameManager->bFever && !GameManager->bIsCleared && !GameManager->bIsGameOver)
			{
				FeverTextBox->SetVisibility(ESlateVisibility::HitTestInvisible);
				const float Hue = FMath::Fmod(Elapsed * 1.5f, 1.0f);
				FeverTextBox->SetColorAndOpacity(FSlateColor(FLinearColor::MakeFromHSV8(static_cast<uint8>(Hue * 255.0f), 255, 255)));
				// 始まった瞬間はドンと大きく出て、すぐ小さく落ち着いてからは脈打つ
				const float Slam = 1.0f + 1.4f * FMath::Exp(-GameManager->FeverAge * 7.0f);
				const float Pulse = Slam * (1.0f + 0.1f * FMath::Sin(Elapsed * 14.0f));
				FeverTextBox->SetRenderScale(FVector2D(Pulse, Pulse));
				FeverTextBox->SetRenderTranslation(FVector2D(0.0f, FMath::Sin(Elapsed * 9.0f) * 5.0f));
				// 白と虹色を行き来する
				const FLinearColor Rainbow = FLinearColor::MakeFromHSV8(static_cast<uint8>(Hue * 255.0f), 200, 255);
				FeverTextBox->SetColorAndOpacity(FSlateColor(FMath::Lerp(FLinearColor::White, Rainbow, 0.5f + 0.5f * FMath::Sin(Elapsed * 8.0f))));
			}
			else
			{
				FeverTextBox->SetVisibility(ESlateVisibility::Collapsed);
			}
		}

		// ステージ開始：STAGE n がドンと出て、READY、GO!!（合計 1.2 秒）
		if (IntroTextBox)
		{
			const float T = GameManager->IntroElapsed;
			const float Duration = GameManager->IntroDuration;
			if (T < Duration)
			{
				IntroTextBox->SetVisibility(ESlateVisibility::HitTestInvisible);
				const float Phase1 = Duration * 0.42f;
				const float Phase2 = Duration * 0.72f;
				float Scale = 1.0f;
				if (T < Phase1)
				{
					IntroTextBox->SetText(FText::FromString(GameManager->StageName));
					IntroTextBox->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.9f, 0.3f)));
					Scale = 1.0f + 2.5f * FMath::Pow(1.0f - FMath::Clamp(T / 0.18f, 0.0f, 1.0f), 2.0f);
				}
				else if (T < Phase2)
				{
					IntroTextBox->SetText(FText::FromString(TEXT("READY")));
					IntroTextBox->SetColorAndOpacity(FSlateColor(FLinearColor::White));
					Scale = 1.0f + 0.5f * FMath::Exp(-(T - Phase1) * 14.0f);
				}
				else
				{
					IntroTextBox->SetText(FText::FromString(TEXT("GO!!")));
					IntroTextBox->SetColorAndOpacity(FSlateColor(FLinearColor(0.3f, 1.0f, 0.5f)));
					Scale = 1.0f + 1.0f * FMath::Exp(-(T - Phase2) * 10.0f);
				}
				IntroTextBox->SetRenderScale(FVector2D(Scale, Scale));
			}
			else
			{
				IntroTextBox->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}
