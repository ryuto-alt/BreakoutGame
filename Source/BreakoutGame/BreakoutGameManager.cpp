#include "BreakoutGameManager.h"

#include "BreakoutBall.h"
#include "BreakoutClearWidget.h"
#include "BreakoutGameInfoWidget.h"
#include "BreakoutGameOverWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ABreakoutGameManager::ABreakoutGameManager()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	BallClass = ABreakoutBall::StaticClass();
	ClearWidgetClass = UBreakoutClearWidget::StaticClass();
	GameOverWidgetClass = UBreakoutGameOverWidget::StaticClass();
	GameInfoWidgetClass = UBreakoutGameInfoWidget::StaticClass();
}

void ABreakoutGameManager::BeginPlay()
{
	Super::BeginPlay();

	// スライドではレベルブループリントの BeginPlay で作っていた「LeftBall」表示
	CreateAndAddWidget(GameInfoWidgetClass);
}

void ABreakoutGameManager::AddBlockNum()
{
	++BlockNum;
	UE_LOG(LogTemp, Log, TEXT("AddBlockNum : %d"), BlockNum);
}

void ABreakoutGameManager::AddBrokenBlockNum()
{
	++BrokenBlockNum;
	UE_LOG(LogTemp, Log, TEXT("BrokenBlockNum : %d"), BrokenBlockNum);
	UE_LOG(LogTemp, Log, TEXT("LeftBlockNum : %d"), GetLeftBlockNum());
	if (GetLeftBlockNum() == 0)
	{
		ViewClearWidget();
		bIsCleared = true;
	}
}

UUserWidget* ABreakoutGameManager::CreateAndAddWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		return nullptr;
	}
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (Widget)
	{
		Widget->AddToViewport();
	}
	return Widget;
}

void ABreakoutGameManager::ViewClearWidget()
{
	CreateAndAddWidget(ClearWidgetClass);
}

void ABreakoutGameManager::ViewGameOverWidget()
{
	CreateAndAddWidget(GameOverWidgetClass);
}

void ABreakoutGameManager::MissCount()
{
	// ボールが消えたので、また発射できる
	bIsBallSpawned = false;
	UE_LOG(LogTemp, Log, TEXT("MissCount : LeftBallNum = %d"), LeftBallNum);

	// 残りがなく、クリアもしていなければゲームオーバー
	if (LeftBallNum <= 0 && !bIsCleared)
	{
		ViewGameOverWidget();
		bIsGameOver = true;
	}
}

void ABreakoutGameManager::SpawnBall()
{
	// 同時に1個まで、残りがあるときだけ
	if (bIsBallSpawned || LeftBallNum <= 0 || !BallClass)
	{
		return;
	}
	const FVector Location = SpawnLocationActor ? SpawnLocationActor->GetActorLocation() : FVector(0.0f, 0.0f, 1000.0f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<ABreakoutBall>(BallClass, FTransform(FRotator::ZeroRotator, Location, FVector::OneVector), Params);
	bIsBallSpawned = true;
	--LeftBallNum;
	UE_LOG(LogTemp, Log, TEXT("SpawnBall : LeftBallNum = %d"), LeftBallNum);
}

void ABreakoutGameManager::Action()
{
	if (bIsGameOver)
	{
		LevelReset();
	}
	else
	{
		SpawnBall();
	}
}

void ABreakoutGameManager::LevelReset()
{
	UE_LOG(LogTemp, Log, TEXT("LevelReset"));
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}
