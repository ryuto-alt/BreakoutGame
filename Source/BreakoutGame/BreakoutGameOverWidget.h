#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BreakoutGameOverWidget.generated.h"

class UTextBlock;

// 「GameOver!」を出す（スライドの GameOverWidget）
UCLASS()
class BREAKOUTGAME_API UBreakoutGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY()
	TObjectPtr<UTextBlock> GameOverBox;
};
