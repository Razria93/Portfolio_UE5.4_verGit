#include "Component/CWeaponComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Weapon/CAttachment.h"
#include "Weapon/CEquipment.h"
#include "Weapon/CAction.h"

UCWeaponComponent::UCWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CurrentWeaponType = EWeaponType::Unarmed;
}

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	// CAttachment
	CreateAttachment(OwnerCharacter_Cached);

	if (IsValid(Attachment))
		Attachment->InitializeAttachment();

	// CEquipment
	CreateEquipment(OwnerCharacter_Cached);

	if (IsValid(Equipment))
		Equipment->InitializeEquipment(OwnerCharacter_Cached, EquipmentData, UnequipmentData);

	// Bind to CEquipment from CAttachment
	if (IsValid(OwnerCharacter_Cached) && IsValid(Attachment) && IsValid(Equipment))
	{
		Equipment->OnEquipmentBeginEquip.AddDynamic(Attachment, &ACAttachment::OnEquipmentBeginEquip);		
		Equipment->OnEquipmentBeginUnequip.AddDynamic(Attachment, &ACAttachment::OnEquipmentBeginUnequip);	
	}

	// CAction
	CreateAction(OwnerCharacter_Cached);

	if (IsValid(Action))
		Action->InitializeAction(OwnerCharacter_Cached, ActionDatas);

	// Bind to CAttachment from CAction
	if (IsValid(OwnerCharacter_Cached) && IsValid(Attachment) && IsValid(Action))
	{
		Attachment->OnAttachmentCollisionEnabled.AddDynamic(Action, &UCAction::OnAttachmentCollisionEnabled);
		Attachment->OnAttachmentCollisionDisabled.AddDynamic(Action, &UCAction::OnAttachmentCollisionDisabled);

		Attachment->OnAttachmentBeginOverlap.AddDynamic(Action, &UCAction::OnAttachmentBeginOverlap);
		Attachment->OnAttachmentEndOverlap.AddDynamic(Action, &UCAction::OnAttachmentEndOverlap);
	}
}

void UCWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Action->Tick(DeltaTime);
}

ACAttachment* UCWeaponComponent::GetAttachment()
{
	return IsValid(Attachment) ? Attachment : nullptr;
}

UCEquipment* UCWeaponComponent::GetEquipment()
{
	return IsValid(Equipment) ? Equipment : nullptr;
}

UCAction* UCWeaponComponent::GetAction()
{
	return IsValid(Action) ? Action : nullptr;
}

void UCWeaponComponent::SetUnarmedMode()
{
	ChangeWeaponMode(EWeaponType::Unarmed);
}

void UCWeaponComponent::SetSwordMode()
{
	ChangeWeaponMode(EWeaponType::Sword);
}

void UCWeaponComponent::PlayAction()
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(Action)) return;

	Action->PlayAction();
}

void UCWeaponComponent::ChangeWeaponMode(EWeaponType InNewWeaponType)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(Equipment)) return;

	if (InNewWeaponType == EWeaponType::Unarmed)
	{
		Equipment->Unequip();
	}
	else
	{
		Equipment->Equip();
	}

	ChangeWeaponType(InNewWeaponType);
}

void UCWeaponComponent::ChangeWeaponType(EWeaponType InNewWeaponType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	EWeaponType prevWeaponType = CurrentWeaponType;
	CurrentWeaponType = InNewWeaponType;

	if (OnWeaponTypeChanged.IsBound())
		OnWeaponTypeChanged.Broadcast(OwnerCharacter_Cached, prevWeaponType, CurrentWeaponType);
}

void UCWeaponComponent::CreateAttachment(AActor* InOwnerCharacter)
{
	if (!IsValid(InOwnerCharacter) || !IsValid(AttachmentClass)) return;

	UWorld* World = InOwnerCharacter->GetWorld();

	if (!World) return;

	// 1) SpawnParams
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = InOwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 2) Spawn Attachment
	Attachment = World->SpawnActor<ACAttachment>(AttachmentClass, SpawnParams);
	check(Attachment);
}

void UCWeaponComponent::CreateEquipment(AActor* InOwnerCharacter)
{
	if (!IsValid(InOwnerCharacter) || !IsValid(EquipmentClass)) return;

	// 1) Create Equipment
	Equipment = NewObject<UCEquipment>(this, EquipmentClass);
	check(Equipment);
}

void UCWeaponComponent::CreateAction(AActor* InOwnerCharacter)
{
	if (!IsValid(InOwnerCharacter) || !IsValid(ActionClass)) return;

	// 1) Create Equipment
	Action = NewObject<UCAction>(this, ActionClass);
	check(Action);
}
