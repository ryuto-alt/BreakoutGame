#include "BreakoutBall.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"

ABreakoutBall::ABreakoutBall()
{
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	RootComponent = Sphere;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Sphere->SetStaticMesh(SphereMesh.Object);
	}
	Sphere->SetCollisionProfileName(TEXT("BlockAll"));
	Sphere->SetMobility(EComponentMobility::Movable);
}

void ABreakoutBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 1フレームで複数回当たっても抜けないよう、残りの移動量で数回だけやり直す
	float Remaining = Speed * DeltaSeconds;
	for (int32 Iteration = 0; Iteration < 3 && Remaining > KINDA_SMALL_NUMBER; ++Iteration)
	{
		FHitResult Hit;
		AddActorWorldOffset(Direction.GetSafeNormal() * Remaining, true, &Hit);
		if (!Hit.bBlockingHit)
		{
			break;
		}
		Remaining *= (1.0f - Hit.Time);
		Bounce(Hit);
		if (!IsValid(this) || IsActorBeingDestroyed())
		{
			return;
		}
	}
}

void ABreakoutBall::Bounce(const FHitResult& Hit)
{
	Direction = UKismetMathLibrary::MirrorVectorByNormal(Direction, Hit.ImpactNormal);
	Direction.X = 0.0f;
}
