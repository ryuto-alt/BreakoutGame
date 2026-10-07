#include "BreakoutBlock.h"

#include "BreakoutAddBallItem.h"
#include "BreakoutGameManager.h"
#include "Engine/World.h"
#include "Math/RandomStream.h"
#include "Misc/CommandLine.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
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
	Cube->SetRelativeScale3D(FVector(1.0f, 5.0f, 1.5f));
	Cube->SetCollisionProfileName(TEXT("BlockAll"));
	ItemClass = ABreakoutAddBallItem::StaticClass();

	// 立方体の手前（カメラ側 = -X）の面に、スケールの影響を受けないよう Root の子として置く
	HpText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HpText"));
	HpText->SetupAttachment(DefaultSceneRoot);
	HpText->SetRelativeLocation(FVector(-51.0f, 0.0f, 0.0f));
	HpText->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	HpText->SetHorizontalAlignment(EHTA_Center);
	HpText->SetVerticalAlignment(EVRTA_TextCenter);
	HpText->SetTextRenderColor(FColor::Black);
	HpText->SetWorldSize(150.0f);
	HpText->SetText(FText::AsNumber(3));
	HpText->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Hp 1〜5：緑、黄、橙、赤、紫（空の青に埋もれない色）
	ColorTable = {
		FColor(70, 210, 90),
		FColor(250, 230, 60),
		FColor(255, 145, 30),
		FColor(235, 50, 50),
		FColor(170, 70, 225),
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

	ReloadHp();

	// GameManager には Block 側から自分の存在を登録する
	if (ABreakoutGameManager* GM = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass())))
	{
		GM->AddBlockNum();
	}
}

void ABreakoutBlock::ReloadHp()
{
	HpText->SetText(FText::AsNumber(FMath::Max(Hp, 0)));
	if (MaterialInstance && ColorTable.IsValidIndex(Hp - 1))
	{
		MaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::FromSRGBColor(ColorTable[Hp - 1]));
	}
}

void ABreakoutBlock::OnBallHit()
{
	--Hp;
	ReloadHp();
	if (Hp <= 0)
	{
		Break();
	}
}

void ABreakoutBlock::Break()
{
	if (ABreakoutGameManager* GM = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass())))
	{
		GM->AddBrokenBlockNum();
	}

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
	if (ItemClass && DropRandom.FRand() < ItemDropRate)
	{
		GetWorld()->SpawnActor<ABreakoutAddBallItem>(ItemClass, GetActorTransform());
		UE_LOG(LogTemp, Log, TEXT("DropItem (%.1f s)"), GetWorld()->GetTimeSeconds());
	}
	Destroy();
}
