#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutAddBallItem.h"
#include "BreakoutGameManager.generated.h"

class UUserWidget;
class USoundBase;
class UAudioComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class ABreakoutBall;
class AStaticMeshActor;
class APostProcessVolume;

// ブロック数・クリア・ゲームオーバー・残ボール・スコア・コンボ・演出を管理する（スライドの「GameManager」）
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

	// ボールを発射位置から扇状に3個生成（場にボールがなく、残りがあるときだけ。残りは1つ減る）
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void SpawnBall();

	// ボールを無条件で1個増やす（アイテム A。LeftBallNum は減らさない）
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void GenerateBall();

	// アイテム S：場にあるボールをそれぞれ2個ずつ増やす（左右反転 + 少しずれた向き）
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void SplitBalls();

	// アイテム P：場にあるボールを数秒間、貫通状態にする
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void PierceBalls();

	// アイテム M：パドルの上から5個を扇状に発射する
	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void MultiBalls();

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

	// ---- 演出 ----
	// ブロックが壊れたとき：スコア（コンボ倍率）・ポップアップ・フラッシュ・かけら・衝撃波・揺れ・ヒットストップ
	void OnBlockBroken(const FVector& Location, const FLinearColor& Color);

	// ブロックに当たっただけのとき：小さな火花とスコア +10
	void OnBlockHit(const FVector& Location, const FLinearColor& Color);

	// 火花を飛ばす（ボールが跳ね返ったとき）
	void SpawnSparks(const FVector& Location, const FVector& Direction, const FLinearColor& Color, int32 Count);

	// 紙吹雪
	void SpawnConfetti(int32 Count, bool bFromTop);

	// コンボに応じて上がる、破壊音のピッチ
	float GetComboPitch() const { return FMath::Min(1.0f + Combo * 0.03f, 1.9f); }

	// 色収差のパルス（大きな出来事で）
	void PulseFringe(float Amount) { FringePulse = FMath::Max(FringePulse, Amount); }

	// 録画用などで、ステージ開始の演出が終わっているか
	UFUNCTION(BlueprintPure, Category = "GameManager")
	bool IsIntroDone() const { return IntroElapsed >= IntroDuration; }

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

	// 連続でブロックを壊した数（ボールを落とすとリセット）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	int32 Combo = 0;

	// FEVER 中（コンボが FeverCombo 以上）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	bool bFever = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameManager")
	int32 FeverCombo = 20;

	// コンボが増えてからの経過秒（HUD のポップ用）
	float ComboPopAge = 10.0f;

	// ステージ開始の演出（STAGE → READY → GO!!）の経過秒と長さ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameManager")
	float IntroElapsed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameManager")
	float IntroDuration = 1.2f;

	// クリアしてからの経過秒（この時間が過ぎるまで Space を受け付けない）
	float ClearedElapsed = 0.0f;

	// クリア後に Space で開くレベル名（レベル上のインスタンスで指定する）。TitleLevel なら最終ステージ
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "GameManager")
	FName NextLevelName;

	// 画面に出すステージ名（例：STAGE 1）
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "GameManager")
	FString StageName = TEXT("STAGE 1");

	// このステージのボールの速さ（ステージが進むほど速い）
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "GameManager")
	float BallSpeed = 1600.0f;

	// 貫通アイテムの効果時間（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameManager")
	float PierceDuration = 5.0f;

	// 同時に存在できるボールの上限（増えすぎ防止）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameManager")
	int32 MaxBallNum = 60;

	UFUNCTION(BlueprintPure, Category = "GameManager")
	bool IsLastStage() const { return NextLevelName == FName(TEXT("TitleLevel")) || NextLevelName.IsNone(); }

	// 演出（かけら・火花・紙吹雪）の材質。BlockMaterial（BaseColor と Glow を持つ）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Effect")
	TObjectPtr<UMaterialInterface> EffectMaterial;

	// BGM（Breakout_BGM）。レベルにドラッグ配置していた SoundCue の代わり
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Sound")
	TObjectPtr<USoundBase> BGMSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Sound")
	float BGMVolume = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Sound")
	TObjectPtr<USoundBase> ClearSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Sound")
	TObjectPtr<USoundBase> GameOverSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Sound")
	TObjectPtr<USoundBase> FeverSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameManager|Sound")
	TObjectPtr<USoundBase> LaunchSound;

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UUserWidget* CreateAndAddWidget(TSubclassOf<UUserWidget> WidgetClass);

	// 指定位置にボールを1個作る（数は InGameBallNum に反映。上限を超えると nullptr）
	ABreakoutBall* SpawnBallAt(const FVector& Location, const FVector& Direction);

	// 扇状にボールを出す
	void LaunchFan(const FVector& Location, int32 Count, float HalfAngleDeg);

	void EndFever();
	void StartHitStop();
	void UpdatePostProcess(float DeltaSeconds);
	void UpdateBackground(float DeltaSeconds);
	void UpdateFeverColors(float DeltaSeconds);

	float ShakeLeft = 0.0f;
	float ShakeDuration = 0.0f;
	float ShakeStrength = 0.0f;
	TWeakObjectPtr<AActor> ShakeTarget;
	FVector ShakeBaseLocation = FVector::ZeroVector;

	// ヒットストップ
	float HitStopLeft = 0.0f;
	float LastHitStopTime = -10.0f;

	// ポストプロセス（ブルーム・色収差・ゲームオーバーの暗転）
	TWeakObjectPtr<APostProcessVolume> PostProcess;
	float BaseBloom = 3.0f;
	float FringePulse = 0.0f;
	float GameOverAlpha = 0.0f;
	float FeverAlpha = 0.0f;
	float FeverHue = 0.0f;

	// 背景
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BackdropMID;

	TArray<TWeakObjectPtr<AStaticMeshActor>> BGLineActors;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> BGLineMIDs;

	TWeakObjectPtr<UAudioComponent> BGMComponent;

	// ALL CLEAR の連続紙吹雪
	int32 ConfettiBurstsLeft = 0;
	float ConfettiTimer = 0.0f;

	// -perflog のときのフレームレート計測
	bool bPerfLog = false;
	float PerfTime = 0.0f;
	int32 PerfFrames = 0;
};
