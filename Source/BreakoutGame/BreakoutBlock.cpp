#include "BreakoutBlock.h"

#include "BreakoutGameManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
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
}

void ABreakoutBlock::BeginPlay()
{
	Super::BeginPlay();

	// GameManager には Block 側から自分の存在を登録する
	if (ABreakoutGameManager* GM = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass())))
	{
		GM->AddBlockNum();
	}
}

void ABreakoutBlock::OnBallHit()
{
	Break();
}

void ABreakoutBlock::Break()
{
	if (ABreakoutGameManager* GM = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass())))
	{
		GM->AddBrokenBlockNum();
	}
	Destroy();
}
