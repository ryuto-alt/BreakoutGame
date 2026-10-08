#include "BreakoutGameManager.h"

#include "BreakoutBall.h"
#include "BreakoutBlock.h"
#include "BreakoutClearWidget.h"
#include "BreakoutDebris.h"
#include "BreakoutGameInfoWidget.h"
#include "BreakoutGameInstance.h"
#include "BreakoutGameOverWidget.h"
#include "BreakoutPopup.h"
#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Sound/SoundBase.h"

namespace
{
	// 色相 0〜1 から鮮やかな色を作る
	FLinearColor Rainbow(float Hue, float Value = 1.0f)
	{
		return FLinearColor::MakeFromHSV8(static_cast<uint8>(FMath::Fmod(Hue, 1.0f) * 255.0f), 255, static_cast<uint8>(FMath::Clamp(Value, 0.0f, 1.0f) * 255.0f));
	}
}

ABreakoutGameManager::ABreakoutGameManager()
{
	PrimaryActorTick.bCanEverTick = true;
	// ヒットストップ中も動かしたいので、時間の流れに左右されにくくはしない（実時間で割って使う）

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	BallClass = ABreakoutBall::StaticClass();
	ClearWidgetClass = UBreakoutClearWidget::StaticClass();
	GameOverWidgetClass = UBreakoutGameOverWidget::StaticClass();
	GameInfoWidgetClass = UBreakoutGameInfoWidget::StaticClass();
}

void ABreakoutGameManager::BeginPlay()
{
	Super::BeginPlay();

	bPerfLog = FParse::Param(FCommandLine::Get(), TEXT("perflog"));

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
		BGMComponent = UGameplayStatics::SpawnSound2D(this, BGMSound, BGMVolume);
	}

	// ポストプロセス（ブルーム・色収差）と背景を探しておく
	for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
	{
		PostProcess = *It;
		BaseBloom = It->Settings.BloomIntensity;
		break;
	}
	for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
	{
		AStaticMeshActor* Actor = *It;
		if (Actor->ActorHasTag(TEXT("Backdrop")))
		{
			BackdropMID = Actor->GetStaticMeshComponent()->CreateDynamicMaterialInstance(0);
		}
		else if (Actor->ActorHasTag(TEXT("BGLine")))
		{
			BGLineActors.Add(Actor);
			BGLineMIDs.Add(Actor->GetStaticMeshComponent()->CreateDynamicMaterialInstance(0));
		}
	}

	// スライドではレベルブループリントの BeginPlay で作っていた「LeftBall」表示
	CreateAndAddWidget(GameInfoWidgetClass);
}

void ABreakoutGameManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// ヒットストップのまま次のレベルへ行かないように戻す
	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
	Super::EndPlay(EndPlayReason);
}

void ABreakoutGameManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float Dilation = FMath::Max(GetWorld()->GetWorldSettings()->GetEffectiveTimeDilation(), 0.01f);
	const float RealDelta = DeltaSeconds / Dilation;

	// ステージ開始の演出、コンボ・クリアの経過時間
	IntroElapsed += RealDelta;
	ComboPopAge += RealDelta;
	if (bIsCleared)
	{
		ClearedElapsed += RealDelta;
	}

	// ヒットストップ（一瞬だけ時間を遅くする）
	if (HitStopLeft > 0.0f)
	{
		HitStopLeft -= RealDelta;
		if (HitStopLeft <= 0.0f)
		{
			UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
		}
	}

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
			ShakeLeft -= RealDelta;
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

	FeverAlpha = FMath::FInterpTo(FeverAlpha, bFever ? 1.0f : 0.0f, RealDelta, 5.0f);
	FeverHue = FMath::Fmod(FeverHue + RealDelta * 0.8f, 1.0f);
	if (bIsGameOver)
	{
		GameOverAlpha = FMath::Min(GameOverAlpha + RealDelta * 1.6f, 1.0f);
	}
	if (BGMComponent.IsValid())
	{
		BGMComponent->SetPitchMultiplier(1.0f + 0.12f * FeverAlpha);
	}

	UpdatePostProcess(RealDelta);
	UpdateBackground(RealDelta);
	UpdateFeverColors(RealDelta);

	// ALL CLEAR の紙吹雪を何回かに分けて出す
	if (ConfettiBurstsLeft > 0)
	{
		ConfettiTimer -= RealDelta;
		if (ConfettiTimer <= 0.0f)
		{
			ConfettiTimer = 0.35f;
			--ConfettiBurstsLeft;
			SpawnConfetti(60, true);
			RequestShake(30.0f, 0.2f);
		}
	}

	// フレームレートの計測（-perflog）
	if (bPerfLog)
	{
		PerfTime += RealDelta;
		++PerfFrames;
		if (PerfTime >= 2.0f)
		{
			UE_LOG(LogTemp, Log, TEXT("PERF fps=%.1f balls=%d debris=%d combo=%d fever=%d actors=%d popups=%d"), PerfFrames / PerfTime, InGameBallNum, ABreakoutDebris::LiveCount, Combo, bFever ? 1 : 0, GetWorld()->GetActorCount(), ABreakoutPopup::LiveCount);
			PerfTime = 0.0f;
			PerfFrames = 0;
		}
	}
}

void ABreakoutGameManager::UpdatePostProcess(float DeltaSeconds)
{
	FringePulse = FMath::Max(0.0f, FringePulse - DeltaSeconds * 5.0f);
	if (!PostProcess.IsValid())
	{
		return;
	}
	FPostProcessSettings& S = PostProcess->Settings;
	S.bOverride_BloomIntensity = true;
	S.BloomIntensity = BaseBloom + 1.5f * FeverAlpha + FringePulse * 0.4f;
	S.bOverride_SceneFringeIntensity = true;
	S.SceneFringeIntensity = FringePulse + 1.2f * FeverAlpha;
	// ゲームオーバーでは色を抜いて暗くする
	S.bOverride_ColorSaturation = true;
	const float Sat = FMath::Lerp(1.0f, 0.12f, GameOverAlpha);
	S.ColorSaturation = FVector4(Sat, Sat, Sat, 1.0f);
	S.bOverride_ColorGain = true;
	const float Gain = FMath::Lerp(1.0f, 0.5f, GameOverAlpha);
	S.ColorGain = FVector4(Gain, Gain, Gain, 1.0f);
}

void ABreakoutGameManager::UpdateBackground(float DeltaSeconds)
{
	const float Time = GetWorld()->GetTimeSeconds();
	const float Beat = 0.5f + 0.5f * FMath::Sin(Time * 2.0f * PI * 2.5f);
	const float ComboFactor = FMath::Clamp(Combo / 30.0f, 0.0f, 1.0f);

	if (BackdropMID)
	{
		FLinearColor Base(0.012f, 0.016f, 0.05f);
		const FLinearColor Tint = FMath::Lerp(FLinearColor(0.05f, 0.1f, 0.4f), Rainbow(FeverHue, 0.6f), FeverAlpha);
		BackdropMID->SetVectorParameterValue(TEXT("BaseColor"), Base + Tint * (0.03f + Beat * (0.04f + 0.2f * ComboFactor)) * (1.0f - 0.7f * FeverAlpha));
	}

	// 横線が下へ流れる（コンボが増えるほど速い）
	const float Speed = 180.0f + ComboFactor * 700.0f + FeverAlpha * 600.0f;
	for (int32 i = 0; i < BGLineActors.Num(); ++i)
	{
		AStaticMeshActor* Line = BGLineActors[i].Get();
		if (!Line)
		{
			continue;
		}
		FVector Location = Line->GetActorLocation();
		Location.Z -= Speed * DeltaSeconds;
		if (Location.Z < -200.0f)
		{
			Location.Z += 5800.0f;
		}
		Line->SetActorLocation(Location);
		if (BGLineMIDs.IsValidIndex(i) && BGLineMIDs[i])
		{
			const FLinearColor Color = FMath::Lerp(FLinearColor(0.08f, 0.2f, 0.9f), Rainbow(FeverHue + i * 0.07f), FeverAlpha);
			BGLineMIDs[i]->SetVectorParameterValue(TEXT("BaseColor"), Color);
			BGLineMIDs[i]->SetScalarParameterValue(TEXT("Glow"), 0.8f + Beat * (0.8f + ComboFactor * 2.0f));
		}
	}
}

void ABreakoutGameManager::UpdateFeverColors(float DeltaSeconds)
{
	if (FeverAlpha < 0.02f && !bFever)
	{
		return;
	}
	// FEVER 中はブロックが虹色に光る
	int32 Index = 0;
	for (TActorIterator<ABreakoutBlock> It(GetWorld()); It; ++It, ++Index)
	{
		if (It->bUnbreakable || !It->MaterialInstance)
		{
			continue;
		}
		if (bFever)
		{
			It->MaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), Rainbow(FeverHue + Index * 0.045f));
			It->MaterialInstance->SetScalarParameterValue(TEXT("Glow"), 3.0f);
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

void ABreakoutGameManager::StartHitStop()
{
	const float Now = GetWorld()->GetRealTimeSeconds();
	// 連続して止まりすぎないように間隔をあける
	if (Now - LastHitStopTime < 0.3f || bIsCleared)
	{
		return;
	}
	LastHitStopTime = Now;
	HitStopLeft = 0.045f;
	UGameplayStatics::SetGlobalTimeDilation(this, 0.06f);
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
	// クリアは派手に：紙吹雪・揺れ・色収差（最終ステージは何度も）
	SpawnConfetti(IsLastStage() ? 90 : 70, true);
	if (IsLastStage())
	{
		ConfettiBurstsLeft = 6;
		ConfettiTimer = 0.4f;
	}
	RequestShake(IsLastStage() ? 70.0f : 50.0f, 0.5f);
	PulseFringe(3.5f);
}

void ABreakoutGameManager::ViewGameOverWidget()
{
	CreateAndAddWidget(GameOverWidgetClass);
	if (GameOverSound)
	{
		UGameplayStatics::PlaySound2D(this, GameOverSound);
	}
	RequestShake(100.0f, 0.7f);
	PulseFringe(4.0f);
}

void ABreakoutGameManager::EndFever()
{
	if (!bFever)
	{
		return;
	}
	bFever = false;
	// ブロックの色を Hp の色に戻す
	for (TActorIterator<ABreakoutBlock> It(GetWorld()); It; ++It)
	{
		It->ReloadHp();
	}
}

void ABreakoutGameManager::MissCount()
{
	// クリア後に落ちたボールは数えない（次のレベルへ持ち越す）
	if (!bIsCleared)
	{
		--InGameBallNum;
		UE_LOG(LogTemp, Log, TEXT("MissCount : InGameBallNum = %d, LeftBallNum = %d"), InGameBallNum, LeftBallNum);

		// ボールを落としたらコンボが途切れる
		Combo = 0;
		EndFever();

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
	if (!BallClass || InGameBallNum >= MaxBallNum)
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

void ABreakoutGameManager::LaunchFan(const FVector& Location, int32 Count, float HalfAngleDeg)
{
	for (int32 i = 0; i < Count; ++i)
	{
		const float Alpha = Count > 1 ? static_cast<float>(i) / (Count - 1) : 0.5f;
		const float Angle = FMath::DegreesToRadians(FMath::Lerp(-HalfAngleDeg, HalfAngleDeg, Alpha) + FMath::FRandRange(-3.0f, 3.0f));
		SpawnBallAt(Location, FVector(0.0f, FMath::Sin(Angle), FMath::Cos(Angle)));
	}
	if (LaunchSound)
	{
		UGameplayStatics::PlaySound2D(this, LaunchSound);
	}
}

void ABreakoutGameManager::SpawnBall()
{
	// ステージ開始の演出が終わっていて、場にボールがなく、残りがあるときだけ
	if (!IsIntroDone() || InGameBallNum != 0 || LeftBallNum <= 0)
	{
		return;
	}
	const FVector Location = SpawnLocationActor ? SpawnLocationActor->GetActorLocation() : FVector(0.0f, 0.0f, 1000.0f);
	LaunchFan(Location, 3, 30.0f);
	--LeftBallNum;
	UE_LOG(LogTemp, Log, TEXT("SpawnBall : LeftBallNum = %d, InGameBallNum = %d"), LeftBallNum, InGameBallNum);
}

void ABreakoutGameManager::GenerateBall()
{
	const FVector Location = SpawnLocationActor ? SpawnLocationActor->GetActorLocation() : FVector(0.0f, 0.0f, 1000.0f);
	const float Angle = FMath::DegreesToRadians(FMath::FRandRange(-35.0f, 35.0f));
	SpawnBallAt(Location, FVector(0.0f, FMath::Sin(Angle), FMath::Cos(Angle)));
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
		// 左右を反転した向きと、少しずらした向きの2個を増やす
		FVector Mirrored(0.0f, -Ball->Direction.Y, Ball->Direction.Z);
		if (FMath::Abs(Mirrored.Y) < 0.25f)
		{
			Mirrored.Y = FMath::RandBool() ? 0.5f : -0.5f;
		}
		const FVector Base = Ball->Direction.GetSafeNormal();
		const float Tilt = FMath::DegreesToRadians(FMath::RandBool() ? 22.0f : -22.0f);
		const FVector Rotated(0.0f, Base.Y * FMath::Cos(Tilt) + Base.Z * FMath::Sin(Tilt), Base.Z * FMath::Cos(Tilt) - Base.Y * FMath::Sin(Tilt));
		const FVector Directions[2] = { Mirrored.GetSafeNormal(), Rotated.GetSafeNormal() };
		for (const FVector& Direction : Directions)
		{
			if (ABreakoutBall* NewBall = SpawnBallAt(Ball->GetActorLocation(), Direction))
			{
				if (Ball->bPiercing)
				{
					NewBall->StartPierce(PierceDuration);
				}
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

void ABreakoutGameManager::MultiBalls()
{
	FVector Location(0.0f, 0.0f, 800.0f);
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Location = Pawn->GetActorLocation() + FVector(0.0f, 0.0f, 130.0f);
	}
	LaunchFan(Location, 5, 42.0f);
	UE_LOG(LogTemp, Log, TEXT("MultiBalls : InGameBallNum = %d"), InGameBallNum);
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
	case EBreakoutItemType::Multi:
		MultiBalls();
		break;
	}
	AddScore(50);
	RequestShake(30.0f, 0.2f);
	PulseFringe(2.0f);
}

void ABreakoutGameManager::Action()
{
	if (bIsGameOver)
	{
		LevelReset();
	}
	else if (bIsCleared)
	{
		// クリア演出のあと、少ししたら次へ進める
		if (ClearedElapsed >= 0.5f)
		{
			OpenNextLevel();
		}
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
		// 持ち越しは最大 30 個まで
		GI->LeftBallNum = FMath::Min(LeftBallNum + InGameBallNum, 30);
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

// ---------------------------------------------------------------- 演出

void ABreakoutGameManager::OnBlockHit(const FVector& Location, const FLinearColor& Color)
{
	AddScore(10);
	SpawnSparks(Location + FVector(-60.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, -1.0f), Color, 5);
}

void ABreakoutGameManager::OnBlockBroken(const FVector& Location, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	++Combo;
	ComboPopAge = 0.0f;

	// スコアはコンボ倍率つき（10 コンボごとに +1 倍）
	const int32 Multiplier = 1 + Combo / 10;
	const int32 Points = 100 * Multiplier;
	AddScore(Points);

	// +100 のポップアップ
	const FColor PopupColor = Multiplier > 1 ? FColor(255, 150, 40) : FColor(255, 240, 80);
	ABreakoutPopup::Spawn(World, Location + FVector(-90.0f, 0.0f, 90.0f), FString::Printf(TEXT("+%d"), Points), PopupColor, 170.0f + 20.0f * FMath::Min(Multiplier, 5));

	// ブロックの形の白いフラッシュ
	ABreakoutDebris::Spawn(World, EffectMaterial, FLinearColor::White, Location + FVector(-70.0f, 0.0f, 0.0f), FVector::ZeroVector, FVector(0.3f, 5.6f, 1.9f), 0.12f, 0.0f, 14.0f, 0.0f);

	// 色つきのかけら
	const int32 DebrisCount = ABreakoutDebris::LiveCount > 260 ? 6 : 16;
	for (int32 i = 0; i < DebrisCount; ++i)
	{
		const FVector Offset(-60.0f, FMath::FRandRange(-230.0f, 230.0f), FMath::FRandRange(-60.0f, 60.0f));
		const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
		const float Speed = FMath::FRandRange(500.0f, 1700.0f);
		const FVector Velocity(0.0f, FMath::Cos(Angle) * Speed, FMath::Sin(Angle) * Speed * 0.9f + 200.0f);
		const FLinearColor Tint = FMath::Lerp(Color, FLinearColor::White, FMath::FRandRange(0.0f, 0.45f));
		ABreakoutDebris::Spawn(World, EffectMaterial, Tint, Location + Offset, Velocity, FVector(FMath::FRandRange(0.22f, 0.5f)), FMath::FRandRange(0.5f, 0.95f), 1700.0f, 4.0f);
	}

	// 衝撃波のリング（外へ広がる火花）
	if (ABreakoutDebris::LiveCount < 330)
	{
		const int32 RingCount = 20;
		for (int32 i = 0; i < RingCount; ++i)
		{
			const float Angle = 2.0f * PI * i / RingCount;
			const FVector Velocity(0.0f, FMath::Cos(Angle) * 1900.0f, FMath::Sin(Angle) * 1900.0f);
			ABreakoutDebris::Spawn(World, EffectMaterial, FMath::Lerp(Color, FLinearColor::White, 0.6f), Location + FVector(-80.0f, 0.0f, 0.0f), Velocity, FVector(0.12f, 0.12f, 0.12f), 0.32f, 0.0f, 8.0f, 0.0f);
		}
	}

	RequestShake(22.0f + FMath::Min(Combo, 30), 0.15f);
	PulseFringe(Combo % 10 == 0 ? 3.0f : 1.2f);
	StartHitStop();

	// 20 コンボで FEVER
	if (!bFever && Combo >= FeverCombo)
	{
		bFever = true;
		UE_LOG(LogTemp, Log, TEXT("FEVER start combo=%d (%.1f s)"), Combo, GetWorld()->GetTimeSeconds());
		if (FeverSound)
		{
			UGameplayStatics::PlaySound2D(this, FeverSound);
		}
		RequestShake(60.0f, 0.4f);
		PulseFringe(4.5f);
		SpawnConfetti(50, true);
	}
}

void ABreakoutGameManager::SpawnSparks(const FVector& Location, const FVector& Direction, const FLinearColor& Color, int32 Count)
{
	if (ABreakoutDebris::LiveCount > 300)
	{
		return;
	}
	for (int32 i = 0; i < Count; ++i)
	{
		const float Spread = FMath::DegreesToRadians(FMath::FRandRange(-70.0f, 70.0f));
		const float Base = FMath::Atan2(Direction.Y, Direction.Z);
		const float Speed = FMath::FRandRange(500.0f, 1500.0f);
		const FVector Velocity(0.0f, FMath::Sin(Base + Spread) * Speed, FMath::Cos(Base + Spread) * Speed);
		ABreakoutDebris::Spawn(GetWorld(), EffectMaterial, Color, Location, Velocity, FVector(FMath::FRandRange(0.1f, 0.2f)), FMath::FRandRange(0.2f, 0.4f), 900.0f, 7.0f, 0.0f);
	}
}

void ABreakoutGameManager::SpawnConfetti(int32 Count, bool bFromTop)
{
	UWorld* World = GetWorld();
	for (int32 i = 0; i < Count; ++i)
	{
		const FVector Location(-80.0f, FMath::FRandRange(-1400.0f, 1400.0f), bFromTop ? FMath::FRandRange(4500.0f, 5000.0f) : 2500.0f);
		const FVector Velocity(0.0f, FMath::FRandRange(-500.0f, 500.0f), FMath::FRandRange(-1200.0f, -200.0f));
		const FLinearColor Color = Rainbow(FMath::FRand());
		ABreakoutDebris::Spawn(World, EffectMaterial, Color, Location, Velocity, FVector(FMath::FRandRange(0.25f, 0.5f), FMath::FRandRange(0.25f, 0.6f), 0.08f), FMath::FRandRange(1.4f, 2.4f), 500.0f, 3.5f, 400.0f);
	}
}
