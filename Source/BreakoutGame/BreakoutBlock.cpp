#include "BreakoutBlock.h"

#include "BreakoutAddBallItem.h"
#include "BreakoutDebris.h"
#include "BreakoutGameManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/RandomStream.h"
#include "Misc/CommandLine.h"
#include "UObject/ConstructorHelpers.h"

ABreakoutBlock::ABreakoutBlock()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	Cube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	Cube->SetupAttachment(DefaultSceneRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Cube->SetStaticMesh(CubeMesh.Object);
	}
	// 横幅 470（隣のブロックとの間に細い隙間をあけて、見分けやすくする）
	Cube->SetRelativeScale3D(FVector(1.0f, 4.7f, 1.5f));
	Cube->SetCollisionProfileName(TEXT("BlockAll"));
	// 壁（WorldStatic）と区別する。貫通ボールは WorldDynamic だけをすり抜ける
	Cube->SetCollisionObjectType(ECC_WorldDynamic);
	ItemClass = ABreakoutAddBallItem::StaticClass();

	// 立方体の手前（カメラ側 = -X）の面に、スケールの影響を受けないよう Root の子として置く
	HpText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HpText"));
	HpText->SetupAttachment(DefaultSceneRoot);
	HpText->SetRelativeLocation(FVector(-52.0f, 0.0f, 0.0f));
	HpText->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	HpText->SetHorizontalAlignment(EHTA_Center);
	HpText->SetVerticalAlignment(EVRTA_TextCenter);
	HpText->SetTextRenderColor(FColor::Black);
	HpText->SetWorldSize(280.0f);
	HpText->SetText(FText::AsNumber(3));
	HpText->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Hp 1〜5：緑、黄、橙、赤、紫（明るく、空や背景に埋もれない色）
	ColorTable = {
		FColor(60, 230, 90),
		FColor(255, 235, 40),
		FColor(255, 140, 20),
		FColor(255, 45, 60),
		FColor(190, 80, 255),
	};
}

void ABreakoutBlock::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// スライドの ConstructionScript：Cube に動的マテリアルを作る
	MaterialInstance = Cube->CreateDynamicMaterialInstance(0, BlockMaterial);
	ReloadHp();
}

void ABreakoutBlock::BeginPlay()
{
	Super::BeginPlay();

	InitialHp = Hp;
	ReloadHp();

	// GameManager には Block 側から自分の存在を登録する（壊れないブロックは数えない）
	if (!bUnbreakable)
	{
		if (ABreakoutGameManager* GM = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass())))
		{
			GM->AddBlockNum();
		}
	}
}

void ABreakoutBlock::ReloadHp()
{
	if (bUnbreakable)
	{
		HpText->SetVisibility(false);
		if (MaterialInstance)
		{
			MaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.16f, 0.17f, 0.2f));
		}
		return;
	}
	HpText->SetVisibility(true);
	HpText->SetText(FText::AsNumber(FMath::Max(Hp, 0)));
	if (MaterialInstance && ColorTable.IsValidIndex(Hp - 1))
	{
		MaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::FromSRGBColor(ColorTable[Hp - 1]));
	}
}

void ABreakoutBlock::OnBallHit()
{
	if (bUnbreakable)
	{
		return;
	}
	--Hp;
	ReloadHp();
	if (ABreakoutGameManager* GM = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass())))
	{
		GM->AddScore(10);
	}
	if (Hp <= 0)
	{
		Break();
	}
}

void ABreakoutBlock::SpawnDebris()
{
	FLinearColor Color = FLinearColor::White;
	if (ColorTable.Num() > 0)
	{
		Color = FLinearColor::FromSRGBColor(ColorTable[FMath::Clamp(InitialHp - 1, 0, ColorTable.Num() - 1)]);
	}
	for (int32 i = 0; i < 16; ++i)
	{
		const FVector Offset(0.0f, FMath::FRandRange(-230.0f, 230.0f), FMath::FRandRange(-60.0f, 60.0f));
		const FVector Velocity(0.0f, FMath::FRandRange(-900.0f, 900.0f), FMath::FRandRange(-200.0f, 1100.0f));
		if (ABreakoutDebris* Debris = GetWorld()->SpawnActor<ABreakoutDebris>(ABreakoutDebris::StaticClass(), GetActorLocation() + Offset, FRotator::ZeroRotator))
		{
			Debris->Init(BlockMaterial, Color, Velocity);
		}
	}
}

void ABreakoutBlock::DropItem()
{
	// 確率でアイテムを落とす（-autoseed のときは同じ結果になるよう乱数を固定）
	static FRandomStream DropRandom;
	static bool bDropRandomInit = false;
	if (!bDropRandomInit)
	{
		bDropRandomInit = true;
		int32 Seed = 0;
		if (FParse::Value(FCommandLine::Get(), TEXT("autoseed="), Seed))
		{
			DropRandom.Initialize(Seed);
		}
		else
		{
			DropRandom.GenerateNewSeed();
		}
	}
	if (!ItemClass || DropRandom.FRand() >= ItemDropRate)
	{
		return;
	}
	// A（ボール追加）/ S（分裂）/ P（貫通）をランダムに
	const int32 TypeIndex = DropRandom.RandRange(0, 2);
	if (ABreakoutAddBallItem* Item = GetWorld()->SpawnActorDeferred<ABreakoutAddBallItem>(ItemClass, GetActorTransform()))
	{
		Item->ItemType = static_cast<EBreakoutItemType>(TypeIndex);
		Item->FinishSpawning(GetActorTransform());
	}
	UE_LOG(LogTemp, Log, TEXT("DropItem type=%d (%.1f s)"), TypeIndex, GetWorld()->GetTimeSeconds());
}

void ABreakoutBlock::Break()
{
	if (ABreakoutGameManager* GM = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass())))
	{
		GM->AddScore(100);
		GM->AddBrokenBlockNum();
		GM->RequestShake(28.0f, 0.15f);
	}
	SpawnDebris();
	if (BreakSound)
	{
		UGameplayStatics::PlaySound2D(this, BreakSound, 1.0f, FMath::FRandRange(0.9f, 1.15f));
	}
	DropItem();
	Destroy();
}
