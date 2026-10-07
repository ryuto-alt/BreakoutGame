#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BreakoutClearWidget.generated.h"

class UTextBlock;

// 「GameClear!!」を出して、ふわっと上下に3回動く（スライドの ClearWidget / FloatAnimation）。
// 下に「Push SPACE to Next Stage」を点滅表示し、最終ステージでは「ALL CLEAR!!」も出す（アレンジ）
UCLASS()
class BREAKOUTGAME_API UBreakoutClearWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 最終ステージのときに呼ぶ（文言を切り替える）
	void SetLastStage(bool bInLast);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY()
	TObjectPtr<UTextBlock> GameClearBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> AllClearBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> PushSpaceTextBox;

	// 1周期の長さ（秒）と繰り返し回数
	UPROPERTY(EditAnywhere, Category = "Float")
	float Duration = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Float")
	int32 NumLoops = 3;

	UPROPERTY(EditAnywhere, Category = "Float")
	float Height = 50.0f;

	float Elapsed = 0.0f;
	bool bLastStage = false;
};
