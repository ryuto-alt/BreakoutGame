#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BreakoutGameInfoWidget.generated.h"

class UTextBlock;
class ABreakoutGameManager;

// 画面上部の表示（スライドの GameInformation）。左上に「LeftBall : n」、中央に STAGE、右上に SCORE（アレンジ）
UCLASS()
class BREAKOUTGAME_API UBreakoutGameInfoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY()
	TObjectPtr<UTextBlock> LeftBallTextBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> StageTextBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> ScoreTextBox;

	// 画面の左右に出すコンボ・FEVER、中央に出すステージ開始の演出
	UPROPERTY()
	TObjectPtr<UTextBlock> ComboTextBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> FeverTextBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> IntroTextBox;

	float Elapsed = 0.0f;

	TWeakObjectPtr<ABreakoutGameManager> GameManager;
};
