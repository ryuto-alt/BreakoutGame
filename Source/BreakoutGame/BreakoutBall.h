#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutBall.generated.h"

class UStaticMeshComponent;
class UPrimitiveComponent;
class USoundBase;

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

	// 反射のたびに鳴らす SE（Breakout_SE_Knock）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball")
	TObjectPtr<USoundBase> KnockSound;

	// 反射後の向きが水平に近すぎないよう、水平からこの角度（度）以上は上を向かせる
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
	float MinHorizontalAngleDeg = 20.0f;

	// 上向きを 0 度とした角度で向きを制限する（スライドの ClampDirection）
	UFUNCTION(BlueprintCallable, Category = "Ball")
	FVector ClampDirection(const FVector& InDirection) const;

	virtual void Tick(float DeltaSeconds) override;

protected:
	// 衝突したときの反射
	virtual void Bounce(const FHitResult& Hit);

	// MissArea に触れたらミスとして数えて消える
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
