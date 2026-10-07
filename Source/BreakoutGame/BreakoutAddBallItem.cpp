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
	Sphere->SetRelativeScale3D(FVector(1.2f));
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
	LabelText->SetWorldSize(60.0f);
	LabelText->SetText(FText::FromString(TEXT("A")));
	LabelText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABreakoutAddBallItem::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInstanceDynamic* MID = Sphere->CreateDynamicMaterialInstance(0, ItemMaterial))
	{
		MID->SetVectorParameterValue(TEXT("BaseColor"), ItemColor);
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
		if (ABreakoutGameManager* GM = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass())))
		{
			GM->GenerateBall();
		}
		Destroy();
	}
}
