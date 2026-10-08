#include "BreakoutAddBallItem.h"

#include "BreakoutGameManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ABreakoutAddBallItem::ABreakoutAddBallItem()
{
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	RootComponent = Sphere;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Sphere->SetStaticMesh(SphereMesh.Object);
	}
	Sphere->SetRelativeScale3D(FVector(1.5f));
	Sphere->SetCollisionProfileName(TEXT("Item"));
	Sphere->SetGenerateOverlapEvents(true);
	Sphere->ComponentTags.Add(TEXT("Item"));
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ABreakoutAddBallItem::OnSphereBeginOverlap);

	// Block の Hp 表示と同じ向き（Yaw 180）で、球の手前に置く
	LabelText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("LabelText"));
	LabelText->SetupAttachment(Sphere);
	LabelText->SetRelativeLocation(FVector(-51.0f, 0.0f, 0.0f));
	LabelText->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	LabelText->SetHorizontalAlignment(EHTA_Center);
	LabelText->SetVerticalAlignment(EVRTA_TextCenter);
	LabelText->SetTextRenderColor(FColor::Black);
	LabelText->SetWorldSize(110.0f);
	LabelText->SetText(FText::FromString(TEXT("A")));
	LabelText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABreakoutAddBallItem::BeginPlay()
{
	Super::BeginPlay();

	// 種類ごとの文字と色
	FLinearColor Color(1.0f, 0.2f, 0.7f);
	FString Letter = TEXT("A");
	if (ItemType == EBreakoutItemType::Split)
	{
		Color = FLinearColor(0.1f, 0.9f, 1.0f);
		Letter = TEXT("S");
	}
	else if (ItemType == EBreakoutItemType::Pierce)
	{
		Color = FLinearColor(1.0f, 0.55f, 0.0f);
		Letter = TEXT("P");
	}
	else if (ItemType == EBreakoutItemType::Multi)
	{
		Color = FLinearColor(0.55f, 0.4f, 1.0f);
		Letter = TEXT("M");
	}
	LabelText->SetText(FText::FromString(Letter));
	if (UMaterialInstanceDynamic* MID = Sphere->CreateDynamicMaterialInstance(0, ItemMaterial))
	{
		MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
		MID->SetScalarParameterValue(TEXT("Glow"), 3.2f);
	}
}

void ABreakoutAddBallItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 真下に落ちる
	FVector Location = GetActorLocation();
	Location.Z -= Speed * DeltaSeconds;
	SetActorLocation(Location);
}

void ABreakoutAddBallItem::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherComp)
	{
		return;
	}
	if (OtherComp->ComponentHasTag(TEXT("MissArea")))
	{
		Destroy();
	}
	else if (OtherComp->ComponentHasTag(TEXT("Player")))
	{
		UE_LOG(LogTemp, Log, TEXT("Pickup type=%d"), static_cast<int32>(ItemType));
		if (ABreakoutGameManager* GM = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass())))
		{
			GM->ApplyItem(ItemType);
		}
		if (PickupSound)
		{
			UGameplayStatics::PlaySound2D(this, PickupSound);
		}
		Destroy();
	}
}
