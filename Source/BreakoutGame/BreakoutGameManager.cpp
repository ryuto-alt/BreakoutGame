#include "BreakoutGameManager.h"

#include "BreakoutBall.h"
#include "BreakoutClearWidget.h"
#include "BreakoutGameInfoWidget.h"
#include "BreakoutGameInstance.h"
#include "BreakoutGameOverWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Sound/SoundBase.h"

ABreakoutGameManager::ABreakoutGameManager()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	BallClass = ABreakoutBall::StaticClass();
	ClearWidgetClass = UBreakoutClearWidget::StaticClass();
	GameOverWidgetClass = UBreakoutGameOverWidget::StaticClass();
	GameInfoWidgetClass = UBreakoutGameInfoWidget::StaticClass();
}

void ABreakoutGameManager::BeginPlay()
{
	Super::BeginPlay();

	// 前のレベルから残りボール数とスコアを受け取る（未設定の -1 のときは既定値のまま）
	if (UBreakoutGameInstance* GI = Cast<UBreakoutGameInstance>(GetGameInstance()))
	{
		Score = GI->Score;
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

void ABreakoutGameManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// カメラの揺れ（だんだん弱くなる）
	if (ShakeLeft > 0.0f)
	{
		if (!ShakeTarget.IsValid())
		{
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				ShakeTarget = PC->GetViewTarget();
			}
			if (ShakeTarget.IsValid())
			{
				ShakeBaseLocation = ShakeTarget->GetActorLocation();
			}
		}
		if (ShakeTarget.IsValid())
		{
			ShakeLeft -= DeltaSeconds;
			if (ShakeLeft <= 0.0f)
			{
				ShakeTarget->SetActorLocation(ShakeBaseLocation);
				ShakeTarget.Reset();
			}
			else
			{
				const float Power = ShakeStrength * (ShakeLeft / ShakeDuration);
				ShakeTarget->SetActorLocation(ShakeBaseLocation + FVector(0.0f, FMath::FRandRange(-Power, Power), FMath::FRandRange(-Power, Power)));
			}
		}
		else
		{
			ShakeLeft = 0.0f;
		}
	}
}

void ABreakoutGameManager::RequestShake(float Strength, float Duration)
{
	// 強い揺れを優先する
	if (Strength * Duration >= ShakeStrength * FMath::Max(ShakeLeft, 0.0f))
	{
		ShakeStrength = Strength;
		ShakeDuration = Duration;
		ShakeLeft = Duration;
	}
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
	if (UBreakoutClearWidget* Widget = Cast<UBreakoutClearWidget>(CreateAndAddWidget(ClearWidgetClass)))
	{
		Widget->SetLastStage(IsLastStage());
	}
	if (ClearSound)
	{
		UGameplayStatics::PlaySound2D(this, ClearSound);
	}
	RequestShake(40.0f, 0.4f);
}

void ABreakoutGameManager::ViewGameOverWidget()
{
	CreateAndAddWidget(GameOverWidgetClass);
	if (GameOverSound)
	{
		UGameplayStatics::PlaySound2D(this, GameOverSound);
	}
	RequestShake(90.0f, 0.6f);
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

ABreakoutBall* ABreakoutGameManager::SpawnBallAt(const FVector& Location, const FVector& Direction)
{
	if (!BallClass)
	{
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ABreakoutBall* Ball = GetWorld()->SpawnActor<ABreakoutBall>(BallClass, FTransform(FRotator::ZeroRotator, Location, FVector::OneVector), Params);
	if (Ball)
	{
		Ball->Speed = BallSpeed;
		Ball->Direction = Direction;
		++InGameBallNum;
	}
	return Ball;
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
	if (InGameBallNum >= MaxBallNum)
	{
		return;
	}
	const FVector Location = SpawnLocationActor ? SpawnLocationActor->GetActorLocation() : FVector(0.0f, 0.0f, 1000.0f);
	SpawnBallAt(Location, FVector(0.0f, 1.0f, 1.0f));
	UE_LOG(LogTemp, Log, TEXT("GenerateBall : InGameBallNum = %d"), InGameBallNum);
}

void ABreakoutGameManager::SplitBalls()
{
	// 生成しながら回ると増えるので、先に今あるボールだけ集める
	TArray<ABreakoutBall*> Balls;
	for (TActorIterator<ABreakoutBall> It(GetWorld()); It; ++It)
	{
		Balls.Add(*It);
	}
	for (ABreakoutBall* Ball : Balls)
	{
		if (InGameBallNum >= MaxBallNum)
		{
			break;
		}
		// 左右を反転した向きにする（ほぼ真上のときは斜めに振る）
		FVector NewDirection(0.0f, -Ball->Direction.Y, Ball->Direction.Z);
		if (FMath::Abs(NewDirection.Y) < 0.25f)
		{
			NewDirection.Y = FMath::RandBool() ? 0.5f : -0.5f;
		}
		if (ABreakoutBall* NewBall = SpawnBallAt(Ball->GetActorLocation(), NewDirection.GetSafeNormal()))
		{
			if (Ball->bPiercing)
			{
				NewBall->StartPierce(PierceDuration);
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("SplitBalls : InGameBallNum = %d"), InGameBallNum);
}

void ABreakoutGameManager::PierceBalls()
{
	for (TActorIterator<ABreakoutBall> It(GetWorld()); It; ++It)
	{
		It->StartPierce(PierceDuration);
	}
	UE_LOG(LogTemp, Log, TEXT("PierceBalls"));
}

void ABreakoutGameManager::ApplyItem(EBreakoutItemType ItemType)
{
	// クリア後に取っても効果はない（持ち越しの数がずれないように）
	if (bIsCleared || bIsGameOver)
	{
		return;
	}
	switch (ItemType)
	{
	case EBreakoutItemType::AddBall:
		GenerateBall();
		break;
	case EBreakoutItemType::Split:
		SplitBalls();
		break;
	case EBreakoutItemType::Pierce:
		PierceBalls();
		break;
	}
	AddScore(50);
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
		GI->Score = Score;
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
		GI->Score = 0;
	}
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}
