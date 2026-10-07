#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BreakoutGameInfoWidget.generated.h"

class UTextBlock;
class ABreakoutGameManager;

// 左上に「LeftBall : n」を出す（スライドの GameInformation）
UCLASS()
class BREAKOUTGAME_API UBreakoutGameInfoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY()
	TObjectPtr<UTextBlock> LeftBallTextBox;

	TWeakObjectPtr<ABreakoutGameManager> GameManager;
};
