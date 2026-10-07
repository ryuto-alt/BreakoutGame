#include "BreakoutTitleManager.h"

#include "BreakoutGameInstance.h"
#include "BreakoutTitleWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/SceneComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"

ABreakoutTitleManager::ABreakoutTitleManager()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	TitleWidgetClass = UBreakoutTitleWidget::StaticClass();
}

void ABreakoutTitleManager::BeginPlay()
{
	Super::BeginPlay();

	bAutoPlay = FParse::Param(FCommandLine::Get(), TEXT("autoplay"));
	ApplyWindowSettings();

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	if (TitleWidgetClass)
	{
		if (UUserWidget* Widget = CreateWidget<UUserWidget>(PC, TitleWidgetClass))
		{
			Widget->AddToViewport();
		}
	}

	// アセットが未設定でも動くように、IA_Action（Space）をその場で作る
	if (!ActionAction)
	{
		ActionAction = NewObject<UInputAction>(this, TEXT("IA_Action"));
		ActionAction->ValueType = EInputActionValueType::Boolean;
	}
	if (!InputMappingContext)
	{
		InputMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Title"));
		InputMappingContext->MapKey(ActionAction, EKeys::SpaceBar);
	}
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}
	EnableInput(PC);
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 押した瞬間だけ
		Input->BindAction(ActionAction, ETriggerEvent::Started, this, &ABreakoutTitleManager::OnAction);
	}
}

void ABreakoutTitleManager::ApplyWindowSettings()
{
	// 録画（-dumpmovie / -uiframes / -benchmark）やエディタでは解像度を変えない
	if (GIsEditor || FParse::Param(FCommandLine::Get(), TEXT("dumpmovie")) || FParse::Param(FCommandLine::Get(), TEXT("uiframes")) || FParse::Param(FCommandLine::Get(), TEXT("benchmark")))
	{
		return;
	}
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		Settings->SetFullscreenMode(EWindowMode::Windowed);
		Settings->SetScreenResolution(FIntPoint(1280, 720));
		Settings->ApplySettings(false);
	}
}

void ABreakoutTitleManager::OnAction(const FInputActionValue& Value)
{
	StartGame();
}

void ABreakoutTitleManager::StartGame()
{
	if (bStarted)
	{
		return;
	}
	bStarted = true;
	// 新しいゲームなので、前のプレイの持ち越しボールは捨てる
	if (UBreakoutGameInstance* GI = Cast<UBreakoutGameInstance>(GetGameInstance()))
	{
		GI->LeftBallNum = -1;
	}
	UE_LOG(LogTemp, Log, TEXT("StartGame : %s"), *NextLevelName.ToString());
	UGameplayStatics::OpenLevel(this, NextLevelName);
}

void ABreakoutTitleManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 録画用：2秒後に自動で Space を押す
	ElapsedTime += DeltaSeconds;
	if (bAutoPlay && ElapsedTime >= 2.0f)
	{
		StartGame();
	}
}
