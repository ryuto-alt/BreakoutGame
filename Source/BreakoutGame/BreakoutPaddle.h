#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "BreakoutPaddle.generated.h"

class UStaticMeshComponent;
class UInputAction;
class UInputMappingContext;
class ABreakoutGameManager;
class UMaterialInterface;

// プレイヤーが操作するパドル（スライドの「Paddle」）。BP の Paddle はこのクラスの子。
UCLASS()
class BREAKOUTGAME_API ABreakoutPaddle : public APawn
{
	GENERATED_BODY()

public:
	ABreakoutPaddle();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paddle")
	TObjectPtr<UStaticMeshComponent> Cube;

	// 左右の移動速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paddle")
	float Speed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	// Space キー（ボール発射・やり直し）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ActionAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paddle")
	TObjectPtr<ABreakoutGameManager> GameManager;

	// BlockMaterial（VectorParameter BaseColor）。パドルの色づけに使う
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Paddle")
	TObjectPtr<UMaterialInterface> PaddleMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Paddle")
	FLinearColor PaddleColor = FLinearColor(0.1f, 0.75f, 1.0f);

	// 天面の端に当たるほど法線を最大この角度まで傾ける（狙い撃ち）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paddle")
	float MaxTiltNormalDeg = 20.0f;

	// 天面に当たったか、反射に使う法線を返す。天面でなければ Normal をそのまま返す
	UFUNCTION(BlueprintCallable, Category = "Paddle")
	void GetTopNormal(const FVector& HitLocation, const FVector& Normal, bool& bIsHitTopSurface, FVector& OutNormal) const;

	// デバッグ：天面の傾いた法線を 21 本の矢印で描く（-debugnormals）
	UFUNCTION(BlueprintCallable, Category = "Paddle")
	void DrawDebugTopSurfaceNormals() const;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Action(const FInputActionValue& Value);

	// 録画用の自動操作（起動オプション -autoplay）。ボールを追いかけて打ち返す
	bool bAutoPlay = false;
	// -automiss: わざとボールから逃げる（ミスのデモ用）
	bool bAutoMiss = false;
	float MissTargetY = 0.0f;
	// 自動プレイの乱数（-autoseed=N で固定。同じ値なら同じ動きになる）
	FRandomStream AutoRandom;
	// 狙い撃ちのデモ：受ける位置を 左端 → 中央 → 右端 と順番に変える
	int32 AimIndex = 0;
	bool bWasDescending = false;
	bool bDebugNormals = false;
	// ボールがない時間（自動で Space を押す用）
	float IdleTime = 0.0f;
	// クリア後の経過時間（自動で次のレベルへ進む用）
	float ClearedTime = 0.0f;
	float AutoAimOffset = 0.0f;
	void UpdateAutoPlay(float DeltaSeconds);

	// アセットが未設定でも動くように、IA_Move / IMC_InGame 相当をその場で作る
	void EnsureInputAssets();
};
