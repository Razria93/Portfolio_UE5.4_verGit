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
	if (CreateAttachment(OwnerCharacter_Cached))
		Attachment->InitializeAttachment();

	// CEquipment
	if (CreateEquipment(OwnerCharacter_Cached))
		Equipment->InitializeEquipment(OwnerCharacter_Cached, EquipmentData, UnequipmentData);

	// Bind to CEquipment from CAttachment
	if (IsValid(OwnerCharacter_Cached) && IsValid(Attachment) && IsValid(Equipment))
	{
		Equipment->OnEquipmentBeginEquip.AddDynamic(Attachment, &ACAttachment::OnEquipmentBeginEquip);
		Equipment->OnEquipmentBeginUnequip.AddDynamic(Attachment, &ACAttachment::OnEquipmentBeginUnequip);
	}

	// CAction
	if (CreateAction(OwnerCharacter_Cached))
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

	if (IsValid(Action))
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

bool UCWeaponComponent::CreateAttachment(AActor* InOwnerCharacter)
{
	if (!IsValid(InOwnerCharacter)) return false;

	checkf(IsValid(AttachmentClass),
		TEXT("UCWeaponComponent: AttachmentClass is not set."));

	UWorld* World = InOwnerCharacter->GetWorld();
	if (!World) return false;

	// 1) SpawnParams
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = InOwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 2) Spawn Attachment
	Attachment = World->SpawnActor<ACAttachment>(AttachmentClass, SpawnParams);

	// 3) Check Attachment Validation
	if (!ensureMsgf(Attachment, TEXT("UCWeaponComponent: Attachment was not created")))
	{
		Attachment = nullptr;
		return false;
	}

	return true;
}

bool UCWeaponComponent::CreateEquipment(AActor* InOwnerCharacter)
{
	if (!IsValid(InOwnerCharacter)) return false;

	checkf(IsValid(EquipmentClass),
		TEXT("UCWeaponComponent: EquipmentClass is not set."));


	// 1) Create Equipment
	Equipment = NewObject<UCEquipment>(this, EquipmentClass);

	// 2) Check Equipment Validation
	if (!ensureMsgf(Equipment, TEXT("UCWeaponComponent: Equipment was not created")))
	{
		Equipment = nullptr;
		return false;
	}

	return true;
}

bool UCWeaponComponent::CreateAction(AActor* InOwnerCharacter)
{
	if (!IsValid(InOwnerCharacter)) return false;

	checkf(IsValid(ActionClass),
		TEXT("UCWeaponComponent: ActionClass is not set."));

	// 1) Create Action
	Action = NewObject<UCAction>(this, ActionClass);

	// 2) Check Action Validation
	if (!ensureMsgf(Action, TEXT("UCWeaponComponent: Action was not created")))
	{
		Action = nullptr;
		return false;
	}

	return true;

}
