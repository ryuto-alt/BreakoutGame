#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BreakoutClearWidget.generated.h"

class UTextBlock;

// 「GameClear!!」を出して、ふわっと上下に3回動く（スライドの ClearWidget / FloatAnimation）
UCLASS()
class BREAKOUTGAME_API UBreakoutClearWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY()
	TObjectPtr<UTextBlock> GameClearBox;

	// 1周期の長さ（秒）と繰り返し回数
	UPROPERTY(EditAnywhere, Category = "Float")
	float Duration = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Float")
	int32 NumLoops = 3;

	UPROPERTY(EditAnywhere, Category = "Float")
	float Height = 50.0f;

	float Elapsed = 0.0f;
};
