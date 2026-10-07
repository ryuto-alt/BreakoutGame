#pragma once

#include "CoreMinimal.h"
#include "BreakoutGameModeBase.h"
#include "BreakoutTitleGameMode.generated.h"

// タイトルレベル用の GameMode。パドルは出さない
UCLASS()
class BREAKOUTGAME_API ABreakoutTitleGameMode : public ABreakoutGameModeBase
{
	GENERATED_BODY()

public:
	ABreakoutTitleGameMode()
	{
		DefaultPawnClass = nullptr;
	}
};
