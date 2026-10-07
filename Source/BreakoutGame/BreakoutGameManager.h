#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutGameManager.generated.h"

class UUserWidget;
class ABreakoutBall;

// ブロック数・クリア・ゲームオーバー・残ボールを管理する（スライドの「GameManager」）
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

	// ボールを発射位置に生成（場にボールがなく、残りがあるときだけ）
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void SpawnBall();

	// Space キー：ゲームオーバーならやり直し、それ以外はボール発射
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void Action();

	UFUNCTION(BlueprintPure, Category = "GameManager")
	int32 GetLeftBallNum() const { return LeftBallNum; }

	// 今のレベルを読み直す
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void LevelReset();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	int32 BlockNum = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	int32 BrokenBlockNum = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	bool bIsCleared = false;

	// ボールの発射位置を示すアクタ（RespawnLocationActor）。レベル上のインスタンスで指定する
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "GameManager")
	TObjectPtr<AActor> SpawnLocationActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	bool bIsBallSpawned = false;

	// 残りのボール数（レベル上で変更できる）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameManager")
	int32 LeftBallNum = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	bool bIsGameOver = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Class")
	TSubclassOf<ABreakoutBall> BallClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Widget")
	TSubclassOf<UUserWidget> ClearWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Widget")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Widget")
	TSubclassOf<UUserWidget> GameInfoWidgetClass;

protected:
	virtual void BeginPlay() override;

	UUserWidget* CreateAndAddWidget(TSubclassOf<UUserWidget> WidgetClass);
};
