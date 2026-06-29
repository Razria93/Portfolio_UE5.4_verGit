#include "Component/CWeaponComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CCombatSignalSourceComponent.h"
#include "Weapon/CWeaponActor.h"

#include "Type/CWeaponStructure.h"

UCWeaponComponent::UCWeaponComponent()
{
}

void UCWeaponComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	CombatSignalSourceComp_Injected = InReferences.CombatSignalSourceComponent;

	ValidateRequiredComponentReferences();

	// CWeaponActor
	CreateWeaponActor(OwnerCharacter_Injected, WeaponActorClassKey, WeaponActorClass);
	CurrentWeaponType = EWeaponType::Unarmed;
}

bool UCWeaponComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ CombatSignalSourceComp_Injected, TEXT("UCCombatSignalSourceComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

void UCWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearRuntimeWeaponState();

	if (IsValid(WeaponActor))
	{
		WeaponActor->Destroy();
		WeaponActor = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

ACWeaponActor* UCWeaponComponent::GetWeaponActor()
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

void UCWeaponComponent::ClearRuntimeWeaponState()
{
	ClearContext();

	if (IsValid(WeaponActor))
	{
		WeaponActor->CollisionDisabled();
		WeaponActor->ToggleTrailActive(false);
	}
}

void UCWeaponComponent::OpenCollisionWindow(FName InCollisionName)
{
	if (!IsValid(WeaponActor)) return;

	WeaponActor->CollisionEnabled(InCollisionName);
}

void UCWeaponComponent::CloseCollisionWindow()
{
	if (!IsValid(WeaponActor)) return;

	WeaponActor->CollisionDisabled();
}

void UCWeaponComponent::ChangeWeaponType(EWeaponType InNewWeaponType)
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	EWeaponType prevWeaponType = CurrentWeaponType;
	CurrentWeaponType = InNewWeaponType;

	if (OnWeaponTypeChanged.IsBound())
		OnWeaponTypeChanged.Broadcast(OwnerCharacter_Injected, prevWeaponType, CurrentWeaponType);
}

FWeaponContext UCWeaponComponent::BuildWeaponContext() const
{
	FWeaponContext weaponContext;

	weaponContext.WeaponType = CurrentWeaponType;

	return weaponContext;
}

FCharacterComponentReferences UCWeaponComponent::BuildWeaponActorReferences() const
{
	FCharacterComponentReferences references;

	references.OwnerCharacter = OwnerCharacter_Injected;
	references.CombatSignalSourceComponent = CombatSignalSourceComp_Injected;

	return references;
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

	const FCharacterComponentReferences references = BuildWeaponActorReferences();
	weaponActor->InitializeReferences(references);
	weaponActor->ApplyInitialWeaponState(InWeaponType);

	WeaponActor = weaponActor;

	return true;
}
