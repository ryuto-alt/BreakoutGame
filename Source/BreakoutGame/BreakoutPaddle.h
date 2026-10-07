#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "BreakoutPaddle.generated.h"

class UStaticMeshComponent;
class UInputAction;
class UInputMappingContext;

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

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);

	// 録画用の自動操作（起動オプション -autoplay）。ボールを追いかけて打ち返す
	bool bAutoPlay = false;
	float AutoAimOffset = 0.0f;
	void UpdateAutoPlay(float DeltaSeconds);

	// アセットが未設定でも動くように、IA_Move / IMC_InGame 相当をその場で作る
	void EnsureInputAssets();
};
