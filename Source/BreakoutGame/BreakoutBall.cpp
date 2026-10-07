#include "BreakoutBall.h"

#include "BreakoutBlock.h"
#include "BreakoutPaddle.h"
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
	// パドルの天面に当たったときだけ、位置に応じて傾けた法線で反射する
	bool bUsedTilt = false;
	if (Hit.GetComponent() && Hit.GetComponent()->ComponentHasTag(TEXT("Player")))
	{
		if (const ABreakoutPaddle* Paddle = Cast<ABreakoutPaddle>(Hit.GetActor()))
		{
			bool bTop = false;
			FVector OutNormal;
			Paddle->GetTopNormal(Hit.ImpactPoint, Hit.ImpactNormal, bTop, OutNormal);
			if (bTop)
			{
				Direction = ClampDirection(UKismetMathLibrary::MirrorVectorByNormal(Direction, OutNormal));
				bUsedTilt = true;
			}
		}
	}
	if (!bUsedTilt)
	{
		Direction = UKismetMathLibrary::MirrorVectorByNormal(Direction, Hit.ImpactNormal);
	}
	Direction.X = 0.0f;

	// 当たった相手が Block なら壊す（Hit イベントに頼らず直接呼ぶ）
	if (ABreakoutBlock* Block = Cast<ABreakoutBlock>(Hit.GetActor()))
	{
		Block->OnBallHit();
	}

	// 当たるたびに SE
	if (KnockSound)
	{
		UGameplayStatics::PlaySound2D(this, KnockSound);
	}
}

FVector ABreakoutBall::ClampDirection(const FVector& InDirection) const
{
	// 縦横を入れ替えて atan2 し、真上を 0 度にする（-180 度付近で逆向きに飛ぶのを防ぐ）
	const float Angle = FMath::RadiansToDegrees(FMath::Atan2(InDirection.Y, InDirection.Z));
	const float Limit = 90.0f - MinHorizontalAngleDeg;
	const float Clamped = FMath::DegreesToRadians(FMath::Clamp(Angle, -Limit, Limit));
	return FVector(0.0f, FMath::Sin(Clamped), FMath::Cos(Clamped));
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
