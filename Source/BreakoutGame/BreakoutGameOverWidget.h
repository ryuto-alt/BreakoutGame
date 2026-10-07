#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BreakoutGameOverWidget.generated.h"

class UTextBlock;

// 「GameOver!」を出す（スライドの GameOverWidget / GameOverAnimation）。
// 白→赤（2秒）に加えて、大きく出てきて縮むポップと揺れ、Push SPACE の点滅（アレンジ）
UCLASS()
class BREAKOUTGAME_API UBreakoutGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY()
	TObjectPtr<UTextBlock> GameOverBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> PushSpaceTextBox;

	// 白→赤にかける秒数
	UPROPERTY(EditAnywhere, Category = "Animation")
	float ColorDuration = 2.0f;

	float Elapsed = 0.0f;
};
