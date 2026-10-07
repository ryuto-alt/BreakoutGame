#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutGameManager.generated.h"

class UUserWidget;

// ブロック数・クリア・ゲームオーバーを管理する（スライドの「GameManager」）
UCLASS()
class BREAKOUTGAME_API ABreakoutGameManager : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutGameManager();

	// Block の BeginPlay から呼ばれる
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void AddBlockNum();

	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void AddBrokenBlockNum();

	UFUNCTION(BlueprintPure, Category = "GameManager")
	int32 GetLeftBlockNum() const { return BlockNum - BrokenBlockNum; }

	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void ViewClearWidget();

	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void ViewGameOverWidget();

	// ボールが MissArea に落ちたとき
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void MissCount();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	int32 BlockNum = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	int32 BrokenBlockNum = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	bool bIsCleared = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Widget")
	TSubclassOf<UUserWidget> ClearWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Widget")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

protected:
	virtual void BeginPlay() override;

	UUserWidget* CreateAndAddWidget(TSubclassOf<UUserWidget> WidgetClass);
};
