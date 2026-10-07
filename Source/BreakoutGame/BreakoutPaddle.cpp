#include "BreakoutPaddle.h"

#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "BreakoutBall.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"

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
	if (!InputMappingContext)
	{
		InputMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_InGame"));
		InputMappingContext->MapKey(MoveAction, EKeys::D);
		FEnhancedActionKeyMapping& Left = InputMappingContext->MapKey(MoveAction, EKeys::A);
		Left.Modifiers.Add(NewObject<UInputModifierNegate>(InputMappingContext));
	}
}

void ABreakoutPaddle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	EnsureInputAssets();
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABreakoutPaddle::Move);
	}
}

void ABreakoutPaddle::Move(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	const FVector Delta = FVector::RightVector * Axis * Speed * GetWorld()->GetDeltaSeconds();
	// Sweep で壁にめり込まないようにする
	AddActorWorldOffset(Delta, true);
}

void ABreakoutPaddle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bAutoPlay)
	{
		UpdateAutoPlay(DeltaSeconds);
	}
}

void ABreakoutPaddle::UpdateAutoPlay(float DeltaSeconds)
{
	// 一番低い位置にあるボールを追う
	const ABreakoutBall* Target = nullptr;
	for (TActorIterator<ABreakoutBall> It(GetWorld()); It; ++It)
	{
		if (!Target || It->GetActorLocation().Z < Target->GetActorLocation().Z)
		{
			Target = *It;
		}
	}
	if (!Target)
	{
		return;
	}
	if (Target->Direction.Z > 0.0f)
	{
		// 上に向かっている間に、次に当てる位置を少しずらしておく
		AutoAimOffset = FMath::FRandRange(-300.0f, 300.0f);
	}
	const float Diff = Target->GetActorLocation().Y + AutoAimOffset - GetActorLocation().Y;
	const float Axis = FMath::Clamp(Diff / 100.0f, -1.0f, 1.0f);
	AddActorWorldOffset(FVector::RightVector * Axis * Speed * DeltaSeconds, true);
}
