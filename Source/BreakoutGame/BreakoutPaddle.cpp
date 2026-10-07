#include "BreakoutPaddle.h"

#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "BreakoutAddBallItem.h"
#include "BreakoutBall.h"
#include "BreakoutGameManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "DrawDebugHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

ABreakoutPaddle::ABreakoutPaddle()
{
	PrimaryActorTick.bCanEverTick = true;

	Cube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	RootComponent = Cube;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Cube->SetStaticMesh(CubeMesh.Object);
	}
	// UE5 の横軸は Y。横長にする
	Cube->SetRelativeScale3D(FVector(1.0f, 10.0f, 1.0f));
	Cube->SetCollisionProfileName(TEXT("Pawn"));
	Cube->ComponentTags.Add(TEXT("Player"));

	AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void ABreakoutPaddle::BeginPlay()
{
	Super::BeginPlay();

	bAutoPlay = FParse::Param(FCommandLine::Get(), TEXT("autoplay"));
	bAutoMiss = FParse::Param(FCommandLine::Get(), TEXT("automiss"));
	bDebugNormals = FParse::Param(FCommandLine::Get(), TEXT("debugnormals"));
	int32 AutoSeed = 1;
	FParse::Value(FCommandLine::Get(), TEXT("autoseed="), AutoSeed);
	AutoRandom.Initialize(AutoSeed);

	GameManager = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass()));

	if (UMaterialInstanceDynamic* MID = Cube->CreateDynamicMaterialInstance(0, PaddleMaterial))
	{
		MID->SetVectorParameterValue(TEXT("BaseColor"), PaddleColor);
	}

	EnsureInputAssets();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void ABreakoutPaddle::EnsureInputAssets()
{
	if (!MoveAction)
	{
		MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move"));
		MoveAction->ValueType = EInputActionValueType::Axis1D;
	}
	if (!ActionAction)
	{
		ActionAction = NewObject<UInputAction>(this, TEXT("IA_Action"));
		ActionAction->ValueType = EInputActionValueType::Boolean;
	}
	if (!InputMappingContext)
	{
		InputMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_InGame"));
		InputMappingContext->MapKey(MoveAction, EKeys::D);
		FEnhancedActionKeyMapping& Left = InputMappingContext->MapKey(MoveAction, EKeys::A);
		Left.Modifiers.Add(NewObject<UInputModifierNegate>(InputMappingContext));
		InputMappingContext->MapKey(ActionAction, EKeys::SpaceBar);
	}
}

void ABreakoutPaddle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	EnsureInputAssets();
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABreakoutPaddle::Move);
		// 押した瞬間だけ（Triggered だと押しっぱなしで連打になる）
		Input->BindAction(ActionAction, ETriggerEvent::Started, this, &ABreakoutPaddle::Action);
	}
}

void ABreakoutPaddle::Move(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	const FVector Delta = FVector::RightVector * Axis * Speed * GetWorld()->GetDeltaSeconds();
	// Sweep で壁にめり込まないようにする
	AddActorWorldOffset(Delta, true);
}

void ABreakoutPaddle::Action(const FInputActionValue& Value)
{
	if (GameManager)
	{
		GameManager->Action();
	}
}

void ABreakoutPaddle::GetTopNormal(const FVector& HitLocation, const FVector& Normal, bool& bIsHitTopSurface, FVector& OutNormal) const
{
	// 上向きの面でなければ（側面など）そのまま返す
	if (Normal.Z <= 0.7f)
	{
		bIsHitTopSurface = false;
		OutNormal = Normal;
		return;
	}
	FVector Origin, BoxExtent;
	GetActorBounds(true, Origin, BoxExtent);
	// 左端 -1、中央 0、右端 +1
	const float T = FMath::Clamp((HitLocation.Y - Origin.Y) / BoxExtent.Y, -1.0f, 1.0f);
	// X 軸まわりに回転。右端ほど +Y 側へ傾け、右で受けたボールが右へ返るようにする
	OutNormal = Normal.RotateAngleAxis(-T * MaxTiltNormalDeg, FVector::ForwardVector);
	bIsHitTopSurface = true;
}

void ABreakoutPaddle::DrawDebugTopSurfaceNormals() const
{
	FVector Origin, BoxExtent;
	GetActorBounds(true, Origin, BoxExtent);
	const int32 NumHalf = 10;
	for (int32 Index = -NumHalf; Index <= NumHalf; ++Index)
	{
		const FVector Start(Origin.X, BoxExtent.Y / NumHalf * Index + Origin.Y, BoxExtent.Z + Origin.Z);
		bool bTop = false;
		FVector Normal;
		GetTopNormal(Start, FVector::UpVector, bTop, Normal);
		DrawDebugDirectionalArrow(GetWorld(), Start, Start + Normal * 200.0f, 100.0f, FColor::Blue, false, 0.0f, 0, 12.0f);
	}
}

void ABreakoutPaddle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDebugNormals)
	{
		DrawDebugTopSurfaceNormals();
	}

	if (bAutoPlay)
	{
		UpdateAutoPlay(DeltaSeconds);
	}
}

namespace
{
	// 落下中のボールが、パドルの高さに届くまでの時間と着地点の Y（壁での跳ね返りを展開して予測）
	void PredictLanding(const ABreakoutBall* Ball, float PaddleZ, float& OutTime, float& OutY)
	{
		const FVector Dir = Ball->Direction.GetSafeNormal();
		const FVector Loc = Ball->GetActorLocation();
		if (Dir.Z >= -0.01f)
		{
			OutTime = 1000.0f;
			OutY = Loc.Y;
			return;
		}
		const float Dz = Loc.Z - PaddleZ;
		OutTime = FMath::Max(Dz, 0.0f) / (-Dir.Z * Ball->Speed);
		const float Unfolded = Loc.Y + (Dir.Y / -Dir.Z) * Dz;
		const float Width = 2900.0f;
		float U = FMath::Fmod(Unfolded + 1450.0f, Width * 2.0f);
		if (U < 0.0f) U += Width * 2.0f;
		if (U > Width) U = Width * 2.0f - U;
		OutY = U - 1450.0f;
	}
}

void ABreakoutPaddle::UpdateAutoPlay(float DeltaSeconds)
{
	// クリアしたら少し待って Space を押す（次のレベルへ）
	if (GameManager && GameManager->bIsCleared)
	{
		ClearedTime += DeltaSeconds;
		if (ClearedTime >= 2.0f)
		{
			ClearedTime = 0.0f;
			GameManager->Action();
			return;
		}
	}

	// いちばん早くパドルの高さに来るボールを狙う（落ちてくるものがなければ一番低いボール）
	const float PaddleZ = GetActorLocation().Z + 100.0f;
	const ABreakoutBall* Target = nullptr;
	float TargetTime = 1000.0f;
	float TargetY = 0.0f;
	for (TActorIterator<ABreakoutBall> It(GetWorld()); It; ++It)
	{
		float Time, Y;
		PredictLanding(*It, PaddleZ, Time, Y);
		if (!Target || Time < TargetTime || (TargetTime >= 1000.0f && Time >= 1000.0f && It->GetActorLocation().Z < Target->GetActorLocation().Z))
		{
			Target = *It;
			TargetTime = Time;
			TargetY = Y;
		}
	}
	if (!Target)
	{
		// ボールがないとき：少し待って Space を押す（ゲームオーバー後は長めに待ってやり直し）
		IdleTime += DeltaSeconds;
		const float Wait = (GameManager && GameManager->bIsGameOver) ? 3.0f : 1.0f;
		if (GameManager && IdleTime >= Wait)
		{
			IdleTime = 0.0f;
			GameManager->Action();
		}
		return;
	}
	IdleTime = 0.0f;

	float DesiredY = TargetY;
	if (bAutoMiss)
	{
		// 落下中のボールの着地点の反対側の端へ逃げる
		if (TargetTime < 1000.0f)
		{
			MissTargetY = TargetY >= 0.0f ? -1000.0f : 1000.0f;
		}
		DesiredY = MissTargetY;
	}
	else
	{
		// 打ち返すたびに、次に受ける位置（パドル上）を 左端 → 中央 → 右端 と変える
		if (Target->Direction.Z < 0.0f)
		{
			bWasDescending = true;
		}
		else if (bWasDescending)
		{
			bWasDescending = false;
			static const float Aim[] = { -430.0f, 0.0f, 430.0f };
			AutoAimOffset = -Aim[++AimIndex % 3];
		}
		DesiredY = (TargetTime < 1000.0f ? TargetY : Target->GetActorLocation().Y) + AutoAimOffset;

		// 落ちてくるアイテムは、ボールに間に合う余裕があるときだけ受けに行く
		const ABreakoutAddBallItem* Item = nullptr;
		for (TActorIterator<ABreakoutAddBallItem> It(GetWorld()); It; ++It)
		{
			if (It->GetActorLocation().Z > PaddleZ && (!Item || It->GetActorLocation().Z < Item->GetActorLocation().Z))
			{
				Item = *It;
			}
		}
		if (Item)
		{
			const float ItemY = Item->GetActorLocation().Y;
			const float ItemTime = (Item->GetActorLocation().Z - PaddleZ) / Item->Speed;
			const float Travel = FMath::Abs(GetActorLocation().Y - ItemY) + FMath::Abs(ItemY - DesiredY);
			const bool bBallSafe = TargetTime >= 1000.0f || TargetTime > Travel / Speed + 0.6f;
			// アイテムがもうすぐ届くときは、ボールより先に間に合うなら受けに行く
			const bool bItemFirst = ItemTime < 1.6f && TargetTime > ItemTime + 0.4f && FMath::Abs(GetActorLocation().Y - ItemY) <= Speed * ItemTime + 100.0f;
			if ((bBallSafe || bItemFirst) && ItemTime > 0.0f)
			{
				DesiredY = ItemY;
			}
		}
	}
	DesiredY = FMath::Clamp(DesiredY, -1000.0f, 1000.0f);
	const float Diff = DesiredY - GetActorLocation().Y;
	const float Axis = FMath::Clamp(Diff / 40.0f, -1.0f, 1.0f);
	AddActorWorldOffset(FVector::RightVector * Axis * Speed * DeltaSeconds, true);
}
