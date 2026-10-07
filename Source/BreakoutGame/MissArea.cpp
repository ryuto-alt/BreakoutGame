#include "MissArea.h"

#include "Components/BoxComponent.h"

AMissArea::AMissArea()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("MissArea"));
	RootComponent = Box;
	Box->SetBoxExtent(FVector(40.0f, 1500.0f, 40.0f));
	// 重なりだけ検知する
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldDynamic);
	Box->SetCollisionResponseToAllChannels(ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);
	Box->ComponentTags.Add(TEXT("MissArea"));
}
