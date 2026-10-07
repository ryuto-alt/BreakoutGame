#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BreakoutGameModeBase.generated.h"

// BP の GameMode「BreakoutGame」の親
UCLASS()
class BREAKOUTGAME_API ABreakoutGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABreakoutGameModeBase();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	// 録画用（起動オプション -uiframes）。UMG も写るように毎フレーム画面を保存する
	bool bUiFrames = false;
	int32 UiFrameIndex = 0;
};
