#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutAddBallItem.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class UPrimitiveComponent;
class UMaterialInterface;

// ブロックから落ちてくる「ボール追加」アイテム（スライドの AddBallItem）。パドルで受けるとボールが1個増える
UCLASS()
class BREAKOUTGAME_API ABreakoutAddBallItem : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutAddBallItem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMeshComponent> Sphere;

	// カメラ側（-X）に「A」を表示する
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTextRenderComponent> LabelText;

	// 落下速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float Speed = 600.0f;

	// BlockMaterial（VectorParameter BaseColor）を使って色を付ける
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UMaterialInterface> ItemMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FLinearColor ItemColor = FLinearColor(0.9f, 0.1f, 0.8f);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
