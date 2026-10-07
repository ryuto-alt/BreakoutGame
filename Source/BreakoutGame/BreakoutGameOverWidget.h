#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BreakoutGameOverWidget.generated.h"

class UTextBlock;

// 「GameOver!」を出し、白から赤へ変える（スライドの GameOverWidget / GameOverAnimation）
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
