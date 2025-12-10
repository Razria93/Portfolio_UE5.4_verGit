#include "Weapon/CAttachment.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"

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

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);
}

void ACAttachment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACAttachment::InitializeAttachment()
{
	AttachToOwnerSocket(SocketName_Holster);
}

void ACAttachment::AttachToOwnerSocket(FName InSocketName)
{
	AttachToComponent(OwnerCharacter_Cached->GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), InSocketName);
}
