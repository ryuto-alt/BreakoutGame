#include "BreakoutGameManager.h"

#include "BreakoutGameOverWidget.h"
#include "BreakoutClearWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ABreakoutGameManager::ABreakoutGameManager()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	ClearWidgetClass = UBreakoutClearWidget::StaticClass();
	GameOverWidgetClass = UBreakoutGameOverWidget::StaticClass();
}

void ABreakoutGameManager::BeginPlay()
{
	Super::BeginPlay();
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
	// クリア後に落ちてもゲームオーバーにしない
	if (!bIsCleared)
	{
		ViewGameOverWidget();
	}
}
