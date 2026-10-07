#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BreakoutTitleWidget.generated.h"

class UTextBlock;

// タイトル画面（スライドの TitleWidget）。大きなタイトルと、点滅する「Push SPACE」
UCLASS()
class BREAKOUTGAME_API UBreakoutTitleWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleTextBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> PushSpaceTextBox;

	float Elapsed = 0.0f;
};
