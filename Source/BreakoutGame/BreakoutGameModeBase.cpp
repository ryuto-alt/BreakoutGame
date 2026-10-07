#include "BreakoutGameModeBase.h"

#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

ABreakoutGameModeBase::ABreakoutGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABreakoutGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	bUiFrames = FParse::Param(FCommandLine::Get(), TEXT("uiframes"));
}

void ABreakoutGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bUiFrames)
	{
		// -dumpmovie は UI を写さないので、UI 付きのスクリーンショットを毎フレーム依頼する
		const FString Path = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Screenshots/UIFrames"), FString::Printf(TEXT("UIFrame%05d"), UiFrameIndex++));
		FScreenshotRequest::RequestScreenshot(Path, true, false, false, FIntRect(), true);
	}
}
