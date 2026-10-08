#include "BreakoutBall.h"

#include "BreakoutBlock.h"
#include "BreakoutGameManager.h"
#include "BreakoutPaddle.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
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
	Sphere->SetCollisionProfileName(TEXT("Ball"));
	Sphere->SetMobility(EComponentMobility::Movable);
	Sphere->SetGenerateOverlapEvents(true);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ABreakoutBall::OnSphereBeginOverlap);

	// 残像：ワールド座標で動かす（ボールには追従させない）
	for (int32 i = 0; i < NumTrail; ++i)
	{
		UStaticMeshComponent* Piece = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Trail%d"), i));
		Piece->SetupAttachment(Sphere);
		Piece->SetAbsolute(true, true, true);
		if (SphereMesh.Succeeded())
		{
			Piece->SetStaticMesh(SphereMesh.Object);
		}
		Piece->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Piece->SetGenerateOverlapEvents(false);
		Piece->SetCastShadow(false);
		Trail.Add(Piece);
	}
}

void ABreakoutBall::BeginPlay()
{
	Super::BeginPlay();

	MaterialInstance = Sphere->CreateDynamicMaterialInstance(0, BallMaterial);
	GameManager = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass()));
	TrailPositions.Init(GetActorLocation(), NumTrail);
	for (int32 i = 0; i < Trail.Num(); ++i)
	{
		Trail[i]->SetMaterial(0, MaterialInstance);
		Trail[i]->SetWorldLocation(GetActorLocation());
		Trail[i]->SetWorldScale3D(FVector(FMath::Max(0.85f - i * 0.13f, 0.1f)));
	}
	UpdateColor();
}

void ABreakoutBall::UpdateColor()
{
	if (MaterialInstance)
	{
		MaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), bPiercing ? FLinearColor(1.0f, 0.18f, 0.02f) : FLinearColor(1.0f, 1.0f, 0.9f));
		MaterialInstance->SetScalarParameterValue(TEXT("Glow"), bPiercing ? 7.0f : 5.0f);
	}
}

void ABreakoutBall::StartPierce(float Duration)
{
	bPiercing = true;
	PierceTimeLeft = Duration;
	// ブロック（WorldDynamic）とは重なるだけにする。壁とパドルでは普通に跳ね返る
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	UpdateColor();
}

void ABreakoutBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SparkCooldown -= DeltaSeconds;

	// FEVER 中は虹色に光る
	const bool bFever = GameManager.IsValid() && GameManager->bFever;
	if (bFever && MaterialInstance)
	{
		const float Hue = FMath::Fmod(GetWorld()->GetTimeSeconds() * 1.2f + GetUniqueID() * 0.013f, 1.0f);
		MaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::MakeFromHSV8(static_cast<uint8>(Hue * 255.0f), 255, 255));
	}
	else if (bWasFever)
	{
		UpdateColor();
	}
	bWasFever = bFever;

	// 残像の位置を1つずつ送る
	TrailPositions.Insert(GetActorLocation(), 0);
	TrailPositions.SetNum(NumTrail, EAllowShrinking::No);
	for (int32 i = 0; i < Trail.Num(); ++i)
	{
		Trail[i]->SetWorldLocation(TrailPositions[i]);
	}

	if (bPiercing)
	{
		PierceTimeLeft -= DeltaSeconds;
		if (PierceTimeLeft <= 0.0f)
		{
			// ブロックの中で元に戻るとはまってしまうので、重なっている間は貫通を続ける
			TArray<AActor*> Overlapping;
			Sphere->GetOverlappingActors(Overlapping, ABreakoutBlock::StaticClass());
			if (Overlapping.Num() == 0)
			{
				bPiercing = false;
				Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
				UpdateColor();
			}
		}
	}

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

void ABreakoutBall::PlayKnock()
{
	// 当たるたびに SE（ピッチを少しずつ変える）
	if (KnockSound)
	{
		UGameplayStatics::PlaySound2D(this, KnockSound, 1.0f, FMath::FRandRange(0.9f, 1.15f));
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
	else if (ABreakoutPaddle* HitPaddle = Cast<ABreakoutPaddle>(Hit.GetActor()))
	{
		// パドルがぷるんと光る
		HitPaddle->OnBallHit();
	}

	// 跳ね返るたびに火花
	if (SparkCooldown <= 0.0f && GameManager.IsValid())
	{
		SparkCooldown = 0.05f;
		const FLinearColor SparkColor = bPiercing ? FLinearColor(1.0f, 0.4f, 0.05f) : FLinearColor(0.8f, 0.95f, 1.0f);
		GameManager->SpawnSparks(Hit.ImpactPoint + FVector(-60.0f, 0.0f, 0.0f), FVector(0.0f, Hit.ImpactNormal.Y, Hit.ImpactNormal.Z), SparkColor, 4);
	}
	Direction.X = 0.0f;

	// 当たった相手が Block なら壊す（Hit イベントに頼らず直接呼ぶ）
	if (ABreakoutBlock* Block = Cast<ABreakoutBlock>(Hit.GetActor()))
	{
		Block->OnBallHit();
	}

	PlayKnock();
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
	if (bPiercing)
	{
		if (ABreakoutBlock* Block = Cast<ABreakoutBlock>(OtherActor))
		{
			if (!Block->bUnbreakable)
			{
				Block->OnBallHit();
				PlayKnock();
			}
			return;
		}
	}
	if (OtherComp && OtherComp->ComponentHasTag(TEXT("MissArea")))
	{
		if (ABreakoutGameManager* GM = Cast<ABreakoutGameManager>(UGameplayStatics::GetActorOfClass(this, ABreakoutGameManager::StaticClass())))
		{
			GM->MissCount();
		}
		Destroy();
	}
}
