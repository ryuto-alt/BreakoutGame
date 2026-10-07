#include "BreakoutGameManager.h"

#include "BreakoutBall.h"
#include "BreakoutClearWidget.h"
#include "BreakoutGameInfoWidget.h"
#include "BreakoutGameInstance.h"
#include "BreakoutGameOverWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Sound/SoundBase.h"

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

	// 前のレベルから残りボール数を受け取る（未設定の -1 のときは既定値のまま）
	if (UBreakoutGameInstance* GI = Cast<UBreakoutGameInstance>(GetGameInstance()))
	{
		if (GI->IsValidBallNum())
		{
			LeftBallNum = GI->LeftBallNum;
		}
		else
		{
			// 録画用：-startballs=5 で最初のレベルの残りボール数を変える（持ち越しを見やすくする）
			int32 StartBalls = 0;
			if (FParse::Value(FCommandLine::Get(), TEXT("startballs="), StartBalls))
			{
				LeftBallNum = StartBalls;
			}
		}
	}

	if (BGMSound)
	{
		UGameplayStatics::SpawnSound2D(this, BGMSound, BGMVolume);
	}

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
	UE_LOG(LogTemp, Log, TEXT("LeftBlockNum : %d (%.1f s)"), GetLeftBlockNum(), GetWorld()->GetTimeSeconds());
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
	// クリア後に落ちたボールは数えない（次のレベルへ持ち越す）
	if (!bIsCleared)
	{
		--InGameBallNum;
		UE_LOG(LogTemp, Log, TEXT("MissCount : InGameBallNum = %d, LeftBallNum = %d"), InGameBallNum, LeftBallNum);

		// 場にも残りにもボールがなければゲームオーバー
		if (LeftBallNum <= 0 && InGameBallNum <= 0)
		{
			ViewGameOverWidget();
			bIsGameOver = true;
		}
	}
}

void ABreakoutGameManager::SpawnBall()
{
	// 場にボールがなく、残りがあるときだけ
	if (InGameBallNum != 0 || LeftBallNum <= 0)
	{
		return;
	}
	GenerateBall();
	--LeftBallNum;
	UE_LOG(LogTemp, Log, TEXT("SpawnBall : LeftBallNum = %d"), LeftBallNum);
}

void ABreakoutGameManager::GenerateBall()
{
	if (!BallClass)
	{
		return;
	}
	const FVector Location = SpawnLocationActor ? SpawnLocationActor->GetActorLocation() : FVector(0.0f, 0.0f, 1000.0f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<ABreakoutBall>(BallClass, FTransform(FRotator::ZeroRotator, Location, FVector::OneVector), Params);
	++InGameBallNum;
	UE_LOG(LogTemp, Log, TEXT("GenerateBall : InGameBallNum = %d"), InGameBallNum);
}

void ABreakoutGameManager::Action()
{
	if (bIsGameOver)
	{
		LevelReset();
	}
	else if (bIsCleared)
	{
		OpenNextLevel();
	}
	else
	{
		SpawnBall();
	}
}

void ABreakoutGameManager::OpenNextLevel()
{
	if (NextLevelName.IsNone())
	{
		return;
	}
	// 場に出ているボールも残りに戻して持ち越す
	if (UBreakoutGameInstance* GI = Cast<UBreakoutGameInstance>(GetGameInstance()))
	{
		GI->LeftBallNum = LeftBallNum + InGameBallNum;
	}
	UE_LOG(LogTemp, Log, TEXT("OpenNextLevel : %s"), *NextLevelName.ToString());
	UGameplayStatics::OpenLevel(this, NextLevelName);
}

void ABreakoutGameManager::LevelReset()
{
	UE_LOG(LogTemp, Log, TEXT("LevelReset"));
	// やり直しは持ち越しをやめて、レベルの既定の残りボール数から始める
	if (UBreakoutGameInstance* GI = Cast<UBreakoutGameInstance>(GetGameInstance()))
	{
		GI->LeftBallNum = -1;
	}
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}
