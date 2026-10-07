#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutDebris.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;

// ブロックが壊れたときに飛び散る小さなかけら（パーティクルの代わり）
UCLASS()
class BREAKOUTGAME_API ABreakoutDebris : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutDebris();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debris")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// 色・材質・初速を決める
	void Init(UMaterialInterface* Material, const FLinearColor& Color, const FVector& InVelocity);

	virtual void Tick(float DeltaSeconds) override;

protected:
	FVector Velocity = FVector::ZeroVector;
	FVector Spin = FVector::ZeroVector;
	float Age = 0.0f;
	float Life = 0.8f;
	float StartScale = 0.25f;
};
