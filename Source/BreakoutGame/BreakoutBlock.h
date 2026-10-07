#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutBlock.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class USoundBase;
class ABreakoutGameManager;
class ABreakoutAddBallItem;

// 壊せるブロック（スライドの「Block」）。耐久値 Hp を持ち、当たるたびに減って色が変わる。
// bUnbreakable のブロックは壊れず、クリア条件にも数えない（アレンジ）
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

	// 残り Hp を表示する文字（カメラ側 = -X の面）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Block")
	TObjectPtr<UTextRenderComponent> HpText;

	// 耐久値（レベル上のインスタンスごとに変えられる）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block", meta = (ClampMin = "1"))
	int32 Hp = 1;

	// 壊れない灰色のブロックにする（数字なし・クリア条件に数えない）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block")
	bool bUnbreakable = false;

	// Hp 1〜5 に対応する色（インデックスは Hp - 1）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	TArray<FColor> ColorTable;

	// VectorParameter「BaseColor」を持つマテリアル（BlockMaterial）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	TObjectPtr<UMaterialInterface> BlockMaterial;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Block")
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;

	// 壊れたときにアイテムを落とす確率（スライドでは Weight 0.5）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ItemDropRate = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	TSubclassOf<ABreakoutAddBallItem> ItemClass;

	// 配置時の Hp（かけらの色に使う）
	int32 InitialHp = 1;

	// 壊れたときの音
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	TObjectPtr<USoundBase> BreakSound;

	// ボールが当たったときに Ball から呼ばれる
	UFUNCTION(BlueprintCallable, Category = "Block")
	void OnBallHit();

	// Hp の表示と色を更新する
	UFUNCTION(BlueprintCallable, Category = "Block")
	void ReloadHp();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;

	// 壊れたときの共通処理（スコア・かけら・音・アイテム）
	void Break();
	void SpawnDebris();
	void DropItem();
};
