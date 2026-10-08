#include "BreakoutPopup.h"

#include "Components/TextRenderComponent.h"
#include "Engine/World.h"

int32 ABreakoutPopup::LiveCount = 0;

ABreakoutPopup::ABreakoutPopup()
{
	PrimaryActorTick.bCanEverTick = true;

	Text = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Text"));
	RootComponent = Text;
	// カメラは -X から見ているので Yaw 180（Block の Hp 表示と同じ向き）
	Text->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	Text->SetHorizontalAlignment(EHTA_Center);
	Text->SetVerticalAlignment(EVRTA_TextCenter);
	Text->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Text->SetCastShadow(false);
}

void ABreakoutPopup::BeginPlay()
{
	Super::BeginPlay();
	++LiveCount;
}

void ABreakoutPopup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	--LiveCount;
	Super::EndPlay(EndPlayReason);
}

ABreakoutPopup* ABreakoutPopup::Spawn(UWorld* World, const FVector& Location, const FString& InText, const FColor& Color, float Size)
{
	if (!World || LiveCount >= 14)
	{
		return nullptr;
	}
	ABreakoutPopup* Popup = World->SpawnActor<ABreakoutPopup>(ABreakoutPopup::StaticClass(), Location, FRotator::ZeroRotator);
	if (Popup)
	{
		Popup->Text->SetText(FText::FromString(InText));
		Popup->Text->SetTextRenderColor(Color);
		Popup->Text->SetWorldSize(Size);
	}
	return Popup;
}

void ABreakoutPopup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Age += DeltaSeconds;
	if (Age >= Life)
	{
		Destroy();
		return;
	}
	// 上へ浮かびながら、ぽんと大きくなってから小さくなって消える
	SetActorLocation(GetActorLocation() + FVector(0.0f, 0.0f, 520.0f * DeltaSeconds));
	const float PopIn = FMath::Clamp(Age / 0.12f, 0.0f, 1.0f);
	const float Pop = 0.4f + 0.9f * PopIn - 0.25f * FMath::Sin(PopIn * PI);
	const float Fade = FMath::Clamp((Life - Age) / 0.3f, 0.0f, 1.0f);
	SetActorScale3D(FVector(Pop * Fade + 0.001f));
}
