#include "Weapon/CAttachment.h"
#include "ProjectGlobal.h"

ACAttachment::ACAttachment()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	check(Root);

	SetRootComponent(Root);
}

void ACAttachment::BeginPlay()
{
	Super::BeginPlay();
}

void ACAttachment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

