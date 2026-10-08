#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutBall.generated.h"

class UStaticMeshComponent;
class UPrimitiveComponent;
class USoundBase;
class UMaterialInterface;
class UMaterialInstanceDynamic;

// 跳ね返るボール（スライドの「Ball」）
UCLASS()
class BREAKOUTGAME_API ABreakoutBall : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutBall();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
	TObjectPtr<UStaticMeshComponent> Sphere;

	// 残像（過去の位置に小さくなっていく球を置く）
	static constexpr int32 NumTrail = 6;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
	TArray<TObjectPtr<UStaticMeshComponent>> Trail;

	// 移動方向（Y=横、Z=縦）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
	FVector Direction = FVector(0.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
	float Speed = 1000.0f;

	// 反射後の向きが水平に近すぎないよう、水平からこの角度（度）以上は上を向かせる
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
	float MinHorizontalAngleDeg = 20.0f;

	// 反射のたびに鳴らす SE（Breakout_SE_Knock）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball")
	TObjectPtr<USoundBase> KnockSound;

	// BlockMaterial（VectorParameter BaseColor）。通常は白、貫通中はオレンジ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball")
	TObjectPtr<UMaterialInterface> BallMaterial;

	// 貫通中（アイテム P）：ブロックをすり抜けて、通るたびにダメージを与える
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
	bool bPiercing = false;

	// 上向きを 0 度とした角度で向きを制限する（スライドの ClampDirection）
	UFUNCTION(BlueprintCallable, Category = "Ball")
	FVector ClampDirection(const FVector& InDirection) const;

	// 指定秒数だけ貫通する
	UFUNCTION(BlueprintCallable, Category = "Ball")
	void StartPierce(float Duration);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	// 衝突したときの反射
	virtual void Bounce(const FHitResult& Hit);

	void UpdateColor();
	void PlayKnock();

	// MissArea に触れたらミスとして数える。貫通中はブロックに触れたらダメージ
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;

	float PierceTimeLeft = 0.0f;

	TArray<FVector> TrailPositions;
	float SparkCooldown = 0.0f;
	TWeakObjectPtr<class ABreakoutGameManager> GameManager;
	bool bWasFever = false;
};
