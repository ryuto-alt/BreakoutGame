#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputActionValue.h"
#include "BreakoutTitleManager.generated.h"

class UUserWidget;
class UInputAction;
class UInputMappingContext;

// タイトルレベルに置くアクタ。TitleWidget を出して、Space で Level1 を開く
// （スライドではレベルブループリントで CreateWidget と SpaceBar キーイベントを置いていた）
UCLASS()
class BREAKOUTGAME_API ABreakoutTitleManager : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutTitleManager();

	// Space を押したときに開くレベル
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	FName NextLevelName = TEXT("Level1");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title")
	TSubclassOf<UUserWidget> TitleWidgetClass;

	// ゲーム本編と同じ IMC_InGame / IA_Action（未設定なら C++ で作る）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> ActionAction;

	UFUNCTION(BlueprintCallable, Category = "Title")
	void StartGame();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	void OnAction(const FInputActionValue& Value);

	// 配布版向け：ウィンドウ 1280x720 に設定する（録画・エディタ起動では行わない）
	void ApplyWindowSettings();

	bool bAutoPlay = false;
	float ElapsedTime = 0.0f;
	bool bStarted = false;
};
