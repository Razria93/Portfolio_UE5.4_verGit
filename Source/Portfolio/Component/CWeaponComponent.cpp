#include "Component/CWeaponComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Weapon/CWeaponActor.h"
#include "Weapon/CEquipment.h"

#include "Type/CWeaponStructure.h"

UCWeaponComponent::UCWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	// CWeaponActor
	CreateWeaponActor(OwnerCharacter_Cached, WeaponType, WeaponActorClass);
	CurrentWeaponType_Cached = EWeaponType::Unarmed;

	// CEquipment
	CreateEquipment(OwnerCharacter_Cached, EquipmentType, EquipmentClass, EquipmentData, UnequipmentData);
	CurrentEquipmentType_Cached = EEquipmentType::None;

	// Bind to CEquipment from CWeaponActor
	if (IsValid(OwnerCharacter_Cached) && IsValid(WeaponActor) && IsValid(Equipment))
	{
		Equipment->OnEquipmentBeginEquip.AddDynamic(WeaponActor, &ACWeaponActor::OnEquipmentBeginEquip);
		Equipment->OnEquipmentBeginUnequip.AddDynamic(WeaponActor, &ACWeaponActor::OnEquipmentBeginUnequip);
	}
}

UObject* UCWeaponComponent::GetWeaponActor()
{
	return IsValid(WeaponActor) ? WeaponActor : nullptr;
}

UObject* UCWeaponComponent::GetEquipment()
{
	return IsValid(Equipment) ? Equipment : nullptr;
}

void UCWeaponComponent::SetUnarmedMode()
{
	ChangeMode(EWeaponType::Unarmed);
}

void UCWeaponComponent::SetSwordMode()
{
	ChangeMode(EWeaponType::Sword);
}

void UCWeaponComponent::PushContextToWeaponActor(const FActionContext& InActionContext)
{
	if (!IsValid(WeaponActor) || !IsValid(Equipment)) return;

	IHitContextProvider* provider = Cast<IHitContextProvider>(WeaponActor);
	if (!provider) return;

	// 1) Build contexts from current state
	const FWeaponContext weaponContext = BuildWeaponContext();
	const FEquipmentContext  equipmentContext = BuildEquipmentContext();

	// 2) Push/Cache into the carrier
	provider->SetLastWeaponContext(weaponContext);
	provider->SetLastEquipmentContext(equipmentContext);
	provider->SetLastActionContext(InActionContext);
}

void UCWeaponComponent::ClearContextToWeaponActor()
{
	if (!IsValid(WeaponActor) || !IsValid(Equipment)) return;

	IHitContextProvider* provider = Cast<IHitContextProvider>(WeaponActor);
	if (!provider) return;

	provider->SetLastOverlapContext(FOverlapContext());
	provider->SetLastWeaponContext(FWeaponContext());
	provider->SetLastEquipmentContext(FEquipmentContext());
	provider->SetLastActionContext(FActionContext());
}

bool UCWeaponComponent::CreateWeaponActor(AActor* InOwnerCharacter, EWeaponType InWeaponType, TSubclassOf<ACWeaponActor> InWeaponActorClass)
{
	if (!IsValid(InOwnerCharacter)) return false;

	if (!ensureMsgf(*InWeaponActorClass, TEXT("UCWeaponComponent: InWeaponActorClass is not set.")))
		return false;

	UWorld* World = InOwnerCharacter->GetWorld();
	if (!World) return false;

	// 1) SpawnParams
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = InOwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 2) Spawn WeaponActor
	// TODO : Deffered Spawn
	ACWeaponActor* weaponActor = World->SpawnActor<ACWeaponActor>(InWeaponActorClass, SpawnParams);

	// 3) Check WeaponActor Validation
	if (!ensureMsgf(IsValid(weaponActor), TEXT("UCWeaponComponent: WeaponActor was not created")))
		return false;

	weaponActor->InitializeWeaponActor(InWeaponType);

	WeaponActor = weaponActor;

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

void UCWeaponComponent::ChangeMode(EWeaponType InNewWeaponType)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(Equipment)) return;

	EWeaponType newWeaponType = InNewWeaponType;
	EEquipmentType newEquipmentType = EEquipmentType::Max;

	switch (newWeaponType)
	{
	case EWeaponType::Unarmed:
		newEquipmentType = EEquipmentType::None;
		Equipment->Unequip();
		break;

	case EWeaponType::Sword:
		newEquipmentType = EEquipmentType::Default;
		Equipment->Equip();
		break;

	default:
		newEquipmentType = EEquipmentType::Max;
	}

	ChangeWeaponType(newWeaponType);
	ChangeEquipmentType(newEquipmentType);
}

void UCWeaponComponent::ChangeWeaponType(EWeaponType InNewWeaponType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	EWeaponType prevWeaponType = CurrentWeaponType_Cached;
	CurrentWeaponType_Cached = InNewWeaponType;

	if (OnWeaponTypeChanged.IsBound())
		OnWeaponTypeChanged.Broadcast(OwnerCharacter_Cached, prevWeaponType, CurrentWeaponType_Cached);
}


void UCWeaponComponent::ChangeEquipmentType(EEquipmentType InNewEquipmentType)
{
	EEquipmentType PrevEquipmentType = CurrentEquipmentType_Cached;
	CurrentEquipmentType_Cached = InNewEquipmentType;

	if (OnEquipmentTypeChanged.IsBound())
		OnEquipmentTypeChanged.Broadcast(OwnerCharacter_Cached, PrevEquipmentType, CurrentEquipmentType_Cached);
}

FWeaponContext UCWeaponComponent::BuildWeaponContext() const
{
	FWeaponContext weaponContext;
	weaponContext.CurrentWeaponType = CurrentWeaponType_Cached;

	return weaponContext;
}

FEquipmentContext UCWeaponComponent::BuildEquipmentContext() const
{
	FEquipmentContext equipmentContext;
	equipmentContext.CurrentEquipmentType = CurrentEquipmentType_Cached;

	return equipmentContext;
}