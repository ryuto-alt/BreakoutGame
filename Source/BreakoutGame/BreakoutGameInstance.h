#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BreakoutGameInstance.generated.h"

// レベルをまたいで残りボール数とスコアを持ち越す（スライドの BreakoutGameInstance）
UCLASS()
class BREAKOUTGAME_API UBreakoutGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// -1 は「まだ設定されていない」。0 = ボールなし と区別するため
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	int32 LeftBallNum = -1;

	UFUNCTION(BlueprintPure, Category = "Game")
	bool IsValidBallNum() const { return LeftBallNum >= 0; }

	// 前のレベルまでのスコア
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	int32 Score = 0;
};
