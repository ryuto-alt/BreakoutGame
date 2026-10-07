#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutAddBallItem.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class UPrimitiveComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class USoundBase;

// アイテムの種類
UENUM(BlueprintType)
enum class EBreakoutItemType : uint8
{
	AddBall UMETA(DisplayName = "A : ボール追加"),
	Split UMETA(DisplayName = "S : 分裂"),
	Pierce UMETA(DisplayName = "P : 貫通"),
};

// ブロックから落ちてくるアイテム（スライドの AddBallItem）。パドルで受けると効果が出る
//   A: ボールが1個増える（スライドどおり）  S: 場のボールがそれぞれ1個ずつ分裂  P: 数秒間ブロックを貫通
UCLASS()
class BREAKOUTGAME_API ABreakoutAddBallItem : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutAddBallItem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMeshComponent> Sphere;

	// カメラ側（-X）に A / S / P を表示する
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTextRenderComponent> LabelText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EBreakoutItemType ItemType = EBreakoutItemType::AddBall;

	// 落下速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float Speed = 600.0f;

	// BlockMaterial（VectorParameter BaseColor）を使って色を付ける
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UMaterialInterface> ItemMaterial;

	// 受け取ったときの音
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<USoundBase> PickupSound;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
