#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutAddBallItem.h"
#include "BreakoutGameManager.generated.h"

class UUserWidget;
class USoundBase;
class ABreakoutBall;

// ブロック数・クリア・ゲームオーバー・残ボール・スコアを管理する（スライドの「GameManager」）
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

	// ボールを無条件で1個増やす（アイテム A。LeftBallNum は減らさない）
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void GenerateBall();

	// アイテム S：場にあるボールをそれぞれもう1個、左右反転した向きで増やす
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void SplitBalls();

	// アイテム P：場にあるボールを数秒間、貫通状態にする
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void PierceBalls();

	// アイテムを受け取ったときの効果
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void ApplyItem(EBreakoutItemType ItemType);

	// Space キー：ゲームオーバーならやり直し、クリア済みなら次へ、それ以外はボール発射
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void Action();

	UFUNCTION(BlueprintPure, Category = "GameManager")
	int32 GetLeftBallNum() const { return LeftBallNum; }

	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void AddScore(int32 Points) { Score += Points; }

	// カメラを少し揺らす（ブロックが壊れたときなど）
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void RequestShake(float Strength, float Duration);

	// クリア後、次のレベルへ（残りボール数とスコアを GameInstance 経由で持ち越す）
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void OpenNextLevel();

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

	// 今レベル内にあるボールの数
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	int32 InGameBallNum = 0;

	// 残りのボール数（レベル上で変更できる）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameManager")
	int32 LeftBallNum = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	bool bIsGameOver = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	int32 Score = 0;

	// クリア後に Space で開くレベル名（レベル上のインスタンスで指定する）。TitleLevel なら最終ステージ
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "GameManager")
	FName NextLevelName;

	// 画面に出すステージ名（例：STAGE 1）
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "GameManager")
	FString StageName = TEXT("STAGE 1");

	// このステージのボールの速さ（ステージが進むほど速い）
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "GameManager")
	float BallSpeed = 1000.0f;

	// 貫通アイテムの効果時間（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameManager")
	float PierceDuration = 5.0f;

	// 同時に存在できるボールの上限（分裂の増えすぎ防止）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameManager")
	int32 MaxBallNum = 10;

	UFUNCTION(BlueprintPure, Category = "GameManager")
	bool IsLastStage() const { return NextLevelName == FName(TEXT("TitleLevel")) || NextLevelName.IsNone(); }

	// BGM（Breakout_BGM）。レベルにドラッグ配置していた SoundCue の代わり
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Sound")
	TObjectPtr<USoundBase> BGMSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Sound")
	float BGMVolume = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Sound")
	TObjectPtr<USoundBase> ClearSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Sound")
	TObjectPtr<USoundBase> GameOverSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Class")
	TSubclassOf<ABreakoutBall> BallClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Widget")
	TSubclassOf<UUserWidget> ClearWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Widget")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Widget")
	TSubclassOf<UUserWidget> GameInfoWidgetClass;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UUserWidget* CreateAndAddWidget(TSubclassOf<UUserWidget> WidgetClass);

	// 指定位置にボールを1個作る（数は InGameBallNum に反映）
	ABreakoutBall* SpawnBallAt(const FVector& Location, const FVector& Direction);

	float ShakeLeft = 0.0f;
	float ShakeDuration = 0.0f;
	float ShakeStrength = 0.0f;
	TWeakObjectPtr<AActor> ShakeTarget;
	FVector ShakeBaseLocation = FVector::ZeroVector;
};
