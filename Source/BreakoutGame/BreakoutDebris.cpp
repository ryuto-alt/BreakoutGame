#include "BreakoutDebris.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ABreakoutDebris::ABreakoutDebris()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCastShadow(false);
	Mesh->SetMobility(EComponentMobility::Movable);
}

void ABreakoutDebris::Init(UMaterialInterface* Material, const FLinearColor& Color, const FVector& InVelocity)
{
	Velocity = InVelocity;
	Spin = FVector(FMath::FRandRange(-720.0f, 720.0f), FMath::FRandRange(-720.0f, 720.0f), FMath::FRandRange(-720.0f, 720.0f));
	StartScale = FMath::FRandRange(0.28f, 0.55f);
	Life = FMath::FRandRange(0.6f, 0.95f);
	Mesh->SetRelativeScale3D(FVector(StartScale));
	if (UMaterialInstanceDynamic* MID = Mesh->CreateDynamicMaterialInstance(0, Material))
	{
		MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
}

void ABreakoutDebris::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Age += DeltaSeconds;
	if (Age >= Life)
	{
		Destroy();
		return;
	}
	// 重力で落ちながら回って、だんだん小さくなる
	Velocity.Z -= 1800.0f * DeltaSeconds;
	SetActorLocation(GetActorLocation() + Velocity * DeltaSeconds);
	AddActorLocalRotation(FRotator(Spin.Y, Spin.Z, Spin.X) * DeltaSeconds);
	Mesh->SetRelativeScale3D(FVector(StartScale * (1.0f - Age / Life)));
}
