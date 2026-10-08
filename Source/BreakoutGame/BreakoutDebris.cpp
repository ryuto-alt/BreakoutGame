#include "BreakoutDebris.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

int32 ABreakoutDebris::LiveCount = 0;

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

void ABreakoutDebris::BeginPlay()
{
	Super::BeginPlay();
	++LiveCount;
}

void ABreakoutDebris::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	--LiveCount;
	Super::EndPlay(EndPlayReason);
}

ABreakoutDebris* ABreakoutDebris::Spawn(UWorld* World, UMaterialInterface* Material, const FLinearColor& Color, const FVector& Location, const FVector& InVelocity,
	const FVector& Scale, float InLife, float InGravity, float Glow, float SpinSpeed)
{
	if (!World || !CanSpawn())
	{
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ABreakoutDebris* Debris = World->SpawnActor<ABreakoutDebris>(ABreakoutDebris::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (!Debris)
	{
		return nullptr;
	}
	Debris->Velocity = InVelocity;
	Debris->Spin = FVector(FMath::FRandRange(-SpinSpeed, SpinSpeed), FMath::FRandRange(-SpinSpeed, SpinSpeed), FMath::FRandRange(-SpinSpeed, SpinSpeed));
	Debris->StartScale = Scale;
	Debris->Life = InLife;
	Debris->Gravity = InGravity;
	Debris->Mesh->SetRelativeScale3D(Scale);
	if (UMaterialInstanceDynamic* MID = Debris->Mesh->CreateDynamicMaterialInstance(0, Material))
	{
		MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
		MID->SetScalarParameterValue(TEXT("Glow"), Glow);
	}
	return Debris;
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
	Velocity.Z -= Gravity * DeltaSeconds;
	SetActorLocation(GetActorLocation() + Velocity * DeltaSeconds);
	if (!Spin.IsNearlyZero())
	{
		AddActorLocalRotation(FRotator(Spin.Y, Spin.Z, Spin.X) * DeltaSeconds);
	}
	const float Remain = 1.0f - Age / Life;
	Mesh->SetRelativeScale3D(StartScale * FMath::Clamp(Remain * 1.6f, 0.0f, 1.0f));
}
