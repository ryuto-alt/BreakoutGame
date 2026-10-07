#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutBlock.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class ABreakoutGameManager;

// 壊せるブロック（スライドの「Block」）
UCLASS()
class BREAKOUTGAME_API ABreakoutBlock : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutBlock();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Block")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Block")
	TObjectPtr<UStaticMeshComponent> Cube;

	// ボールが当たったときに Ball から呼ばれる
	UFUNCTION(BlueprintCallable, Category = "Block")
	void OnBallHit();

protected:
	virtual void BeginPlay() override;

	// 壊れたときの共通処理
	void Break();
};
