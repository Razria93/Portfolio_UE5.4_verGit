#include "Component/CWeaponComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Weapon/CAttachment.h"
#include "Weapon/CEquipment.h"

#include "Type/CWeaponStructure.h"

UCWeaponComponent::UCWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	// CAttachment
	CreateAttachment(OwnerCharacter_Cached, AttachmentType, AttachmentClass);
	CurrentAttachmentType_Cached = EAttachmentType::Unarmed;

	// CEquipment
	CreateEquipment(OwnerCharacter_Cached, EquipmentType, EquipmentClass, EquipmentData, UnequipmentData);
	CurrentEquipmentType_Cached = EEquipmentType::None;

	// Bind to CEquipment from CAttachment
	if (IsValid(OwnerCharacter_Cached) && IsValid(Attachment) && IsValid(Equipment))
	{
		Equipment->OnEquipmentBeginEquip.AddDynamic(Attachment, &ACAttachment::OnEquipmentBeginEquip);
		Equipment->OnEquipmentBeginUnequip.AddDynamic(Attachment, &ACAttachment::OnEquipmentBeginUnequip);
	}
}

void UCWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

ACAttachment* UCWeaponComponent::GetAttachment()
{
	return IsValid(Attachment) ? Attachment : nullptr;
}

UCEquipment* UCWeaponComponent::GetEquipment()
{
	return IsValid(Equipment) ? Equipment : nullptr;
}

void UCWeaponComponent::SetUnarmedMode()
{
	ChangeAttachmentMode(EAttachmentType::Unarmed);
}

void UCWeaponComponent::SetSwordMode()
{
	ChangeAttachmentMode(EAttachmentType::Sword);
}

void UCWeaponComponent::ChangeAttachmentMode(EAttachmentType InNewAttachmentType)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(Equipment)) return;

	if (InNewAttachmentType == EAttachmentType::Unarmed)
	{
		Equipment->Unequip();
	}
	else
	{
		Equipment->Equip();
	}

	ChangeAttachmentType(InNewAttachmentType);
}

void UCWeaponComponent::ChangeAttachmentType(EAttachmentType InNewAttachmentType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	EAttachmentType prevAttachmentType = CurrentAttachmentType_Cached;
	CurrentAttachmentType_Cached = InNewAttachmentType;

	if (OnAttachmentTypeChanged.IsBound())
		OnAttachmentTypeChanged.Broadcast(OwnerCharacter_Cached, prevAttachmentType, CurrentAttachmentType_Cached);
}

bool UCWeaponComponent::CreateAttachment(AActor* InOwnerCharacter, EAttachmentType InAttachmentType, TSubclassOf<ACAttachment> InAttachmentClass)
{
	if (!IsValid(InOwnerCharacter)) return false;

	if (!ensureMsgf(*InAttachmentClass, TEXT("UCWeaponComponent: InAttachmentClass is not set.")))
		return false;

	UWorld* World = InOwnerCharacter->GetWorld();
	if (!World) return false;

	// 1) SpawnParams
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = InOwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 2) Spawn Attachment
	// TODO : Deffered Spawn
	ACAttachment* attachment = World->SpawnActor<ACAttachment>(InAttachmentClass, SpawnParams);

	// 3) Check Attachment Validation
	if (!ensureMsgf(IsValid(attachment), TEXT("UCWeaponComponent: Attachment was not created")))
		return false;

	attachment->InitializeAttachment(InAttachmentType);

	Attachment = attachment;

	return true;
}

bool UCWeaponComponent::CreateEquipment(AActor* InOwnerCharacter, EEquipmentType InEquipmentType, TSubclassOf<UCEquipment> InEquipmentClass, const FEquipmentData& InEquipmentDatas, const FEquipmentData& InUnequipmentDatas)
{
	if (!IsValid(InOwnerCharacter)) return false;

	if (!ensureMsgf(*InEquipmentClass, TEXT("UCWeaponComponent: InEquipmentClass is not set.")))
		return false;

	// 1) Create Equipment
	UCEquipment* equipment = NewObject<UCEquipment>(this, InEquipmentClass);

	// 2) Check Equipment Validation
	if (!ensureMsgf(IsValid(equipment), TEXT("UCWeaponComponent: Equipment was not created.")))
		return false;

	ACharacter* character = Cast<ACharacter>(InOwnerCharacter);

	if (!ensureMsgf(IsValid(character), TEXT("UCActionComponent:InOwnerCharacter cast failed.")))
		return false;

	equipment->InitializeEquipment(character, InEquipmentType, InEquipmentDatas, InUnequipmentDatas);

	Equipment = equipment;

	return true;
}