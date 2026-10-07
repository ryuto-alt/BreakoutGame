#include "BreakoutBall.h"

#include "BreakoutBlock.h"
#include "BreakoutGameManager.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
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
	Sphere->SetGenerateOverlapEvents(true);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ABreakoutBall::OnSphereBeginOverlap);
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

	// 当たった相手が Block なら壊す（Hit イベントに頼らず直接呼ぶ）
	if (ABreakoutBlock* Block = Cast<ABreakoutBlock>(Hit.GetActor()))
	{
		Block->OnBallHit();
	}
}

void ABreakoutBall::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp && OtherComp->ComponentHasTag(TEXT("MissArea")))
	{
		if (ABreakoutGameManager* GM = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass())))
		{
			GM->MissCount();
		}
		Destroy();
	}
}
