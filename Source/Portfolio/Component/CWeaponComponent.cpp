#include "Component/CWeaponComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Weapon/CAttachment.h"

UCWeaponComponent::UCWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	CreateAttachment();

	if (IsValid(Attachment))
		Attachment->InitializeAttachment();
}

void UCWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCWeaponComponent::CreateAttachment()
{
	if (!IsValid(OwnerCharacter_Cached) || !AttachmentClass)
		return;

	UWorld* World = OwnerCharacter_Cached->GetWorld();

	if (!World)
		return;

	// 1) SpawnParams
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerCharacter_Cached;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 2) Spawn Attachment
	Attachment = World->SpawnActor<ACAttachment>(AttachmentClass, SpawnParams);
}

