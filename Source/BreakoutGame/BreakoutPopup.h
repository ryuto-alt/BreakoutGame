#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakoutPopup.generated.h"

class UTextRenderComponent;

// 「+100」のようにブロックの位置から浮かび上がって消える文字（スコアポップアップ）
UCLASS()
class BREAKOUTGAME_API ABreakoutPopup : public AActor
{
	GENERATED_BODY()

public:
	ABreakoutPopup();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Popup")
	TObjectPtr<UTextRenderComponent> Text;

	static int32 LiveCount;

	static ABreakoutPopup* Spawn(UWorld* World, const FVector& Location, const FString& InText, const FColor& Color, float Size);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	float Age = 0.0f;
	float Life = 0.85f;
};
