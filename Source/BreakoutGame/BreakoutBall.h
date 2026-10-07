#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutBall.generated.h"

class UStaticMeshComponent;

// 跳ね返るボール（スライドの「Ball」）
UCLASS()
class BREAKOUTGAME_API ABreakoutBall : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutBall();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
	TObjectPtr<UStaticMeshComponent> Sphere;

	// 移動方向（Y=横、Z=縦）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
	FVector Direction = FVector(0.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
	float Speed = 1000.0f;

	virtual void Tick(float DeltaSeconds) override;

protected:
	// 衝突したときの反射
	virtual void Bounce(const FHitResult& Hit);
};
