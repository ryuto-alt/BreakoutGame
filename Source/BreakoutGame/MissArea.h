#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MissArea.generated.h"

class UBoxComponent;

// 下端でボールを検知するトリガー（スライドの「MissArea」）
UCLASS()
class BREAKOUTGAME_API AMissArea : public AActor
{
	GENERATED_BODY()

public:
	AMissArea();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MissArea")
	TObjectPtr<UBoxComponent> Box;
};
