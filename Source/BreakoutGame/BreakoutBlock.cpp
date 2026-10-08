#include "BreakoutBlock.h"

#include "BreakoutAddBallItem.h"
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
			MaterialInstance->SetScalarParameterValue(TEXT("Glow"), 0.9f);
		}
		return;
	}
	HpText->SetVisibility(true);
	HpText->SetText(FText::AsNumber(FMath::Max(Hp, 0)));
	if (MaterialInstance && ColorTable.IsValidIndex(Hp - 1))
	{
		MaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::FromSRGBColor(ColorTable[Hp - 1]));
		MaterialInstance->SetScalarParameterValue(TEXT("Glow"), 1.15f);
	}
}

void ABreakoutBlock::OnBallHit()
{
	if (bUnbreakable)
	{
		return;
	}
	const FLinearColor HitColor = GetBaseColor();
	--Hp;
	ReloadHp();
	if (ABreakoutGameManager* GM = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass())))
	{
		GM->OnBlockHit(GetActorLocation(), HitColor);
	}
	if (Hp <= 0)
	{
		Break();
	}
	else if (MaterialInstance)
	{
		// 当たった瞬間だけ白く光る
		MaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
		MaterialInstance->SetScalarParameterValue(TEXT("Glow"), 9.0f);
		GetWorldTimerManager().SetTimer(FlashTimer, FTimerDelegate::CreateWeakLambda(this, [this]() { ReloadHp(); }), 0.06f, false);
	}
}

FLinearColor ABreakoutBlock::GetBaseColor() const
{
	if (ColorTable.IsValidIndex(InitialHp - 1))
	{
		return FLinearColor::FromSRGBColor(ColorTable[InitialHp - 1]);
	}
	return FLinearColor::White;
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
	// 録画用：-noitems でアイテムを落とさない
	if (!ItemClass || FParse::Param(FCommandLine::Get(), TEXT("noitems")) || DropRandom.FRand() >= ItemDropRate)
	{
		return;
	}
	// A（ボール追加）/ S（分裂）/ P（貫通）/ M（5個発射）をランダムに
	const int32 TypeIndex = DropRandom.RandRange(0, 3);
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
		GM->AddBrokenBlockNum();
		GM->OnBlockBroken(GetActorLocation(), GetBaseColor());
		if (BreakSound)
		{
			// コンボが増えるほど音が高くなる
			UGameplayStatics::PlaySound2D(this, BreakSound, 1.0f, GM->GetComboPitch());
		}
	}
	DropItem();
	Destroy();
}
