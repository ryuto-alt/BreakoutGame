#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutDebris.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;

// かけら・火花・紙吹雪・フラッシュなどの小さな演出用アクタ（パーティクルの代わり）
UCLASS()
class BREAKOUTGAME_API ABreakoutDebris : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutDebris();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debris")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// 同時に存在できる数の上限（増えすぎて重くならないように）
	static constexpr int32 MaxLive = 420;
	static int32 LiveCount;
	static bool CanSpawn(int32 Num = 1) { return LiveCount + Num <= MaxLive; }

	// 生成する。上限を超えるときは nullptr
	static ABreakoutDebris* Spawn(UWorld* World, UMaterialInterface* Material, const FLinearColor& Color, const FVector& Location, const FVector& Velocity,
		const FVector& Scale, float Life, float Gravity, float Glow, float SpinSpeed = 600.0f);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FVector Velocity = FVector::ZeroVector;
	FVector Spin = FVector::ZeroVector;
	FVector StartScale = FVector(0.25f);
	float Age = 0.0f;
	float Life = 0.8f;
	float Gravity = 1800.0f;
};
