#include "Component/CWeaponComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Weapon/CWeaponActor.h"

#include "Type/CWeaponStructure.h"

UCWeaponComponent::UCWeaponComponent()
{
}

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	// CWeaponActor
	CreateWeaponActor(OwnerCharacter_Cached, WeaponActorClassKey, WeaponActorClass);
	CurrentWeaponType = EWeaponType::Unarmed;
}

UObject* UCWeaponComponent::GetWeaponActor()
{
	return IsValid(WeaponActor) ? WeaponActor : nullptr;
}

void UCWeaponComponent::AttachWeaponToHand()
{
	if (!IsValid(WeaponActor)) return;

	WeaponActor->AttachToHandSocket();
}

void UCWeaponComponent::AttachWeaponToHolster()
{
	if (!IsValid(WeaponActor)) return;

	WeaponActor->AttachToHolsterSocket();
}

void UCWeaponComponent::CommitEquipWeapon()
{
	if (!IsValid(WeaponActor)) return;

	ChangeWeaponType(WeaponActor->GetWeaponType());
}

void UCWeaponComponent::CommitUnequipWeapon()
{
	ChangeWeaponType(EWeaponType::Unarmed);
}

void UCWeaponComponent::PushContext(const FActionContext& InActionContext)
{
	if (!IsValid(WeaponActor)) return;

	IHitContextProvider* provider = Cast<IHitContextProvider>(WeaponActor);
	if (!provider) return;

	// 1) Build contexts from current state
	const FWeaponContext weaponContext = BuildWeaponContext();

	// 2) Push/Cache into the carrier
	provider->SetLastWeaponContext(weaponContext);
	provider->SetLastActionContext(InActionContext);
}

void UCWeaponComponent::ClearContext()
{
	if (!IsValid(WeaponActor)) return;

	IHitContextProvider* provider = Cast<IHitContextProvider>(WeaponActor);
	if (!provider) return;

	provider->SetLastOverlapContext(FOverlapContext());
	provider->SetLastWeaponContext(FWeaponContext());
	provider->SetLastActionContext(FActionContext());
}

void UCWeaponComponent::ChangeWeaponType(EWeaponType InNewWeaponType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	EWeaponType prevWeaponType = CurrentWeaponType;
	CurrentWeaponType = InNewWeaponType;

	if (OnWeaponTypeChanged.IsBound())
		OnWeaponTypeChanged.Broadcast(OwnerCharacter_Cached, prevWeaponType, CurrentWeaponType);
}

FWeaponContext UCWeaponComponent::BuildWeaponContext() const
{
	FWeaponContext weaponContext;

	weaponContext.WeaponType = CurrentWeaponType;

	return weaponContext;
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