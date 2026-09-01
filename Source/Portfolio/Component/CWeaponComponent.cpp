#include "Component/CWeaponComponent.h"

#include "ProjectGlobal.h"

#include "Component/CCombatSignalSourceComponent.h"
#include "Core/Profiling/CCombatCollisionProfiling.h"
#include "Core/Profiling/CCombatCollisionProfilingCounters.h"
#include "Weapon/CWeaponActor.h"
#include "Type/CWeaponTypes.h"
#include "Type/CCombatHitTypes.h"

#include "GameFramework/Character.h"

UCWeaponComponent::UCWeaponComponent()
{
}

// Component Reference

void UCWeaponComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	CombatSignalSourceComp_Injected = InReferences.CombatSignalSourceComponent;

	ValidateRequiredComponentReferences();

	CurrentWeaponType = EWeaponType::Unarmed;
	bWeaponActorDisabledForProfiling = false;

	if (ShouldSkipWeaponActorCreationForProfiling())
	{
		SkipWeaponActorCreationForProfiling();
		return;
	}

	CreateWeaponActor(OwnerCharacter_Injected, WeaponActorClassKey, WeaponActorClass);
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

// Lifecycle

void UCWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeWeaponRuntime();

	Super::EndPlay(EndPlayReason);
}

// Runtime Lifecycle

void UCWeaponComponent::UninitializeWeaponRuntime()
{
	ClearWeaponRuntimeState();
	DestroyWeaponActor();
}

// Query

ACWeaponActor* UCWeaponComponent::GetWeaponActor() const
{
	return IsValid(WeaponActor) ? WeaponActor : nullptr;
}

// Mutation

void UCWeaponComponent::AttachWeaponToHand()
{
	if (!IsValid(WeaponActor)) return;

	ClearWeaponPresentationOverride();
	WeaponActor->AttachToHandSocket();
	CaptureWeaponAttachmentRelativeTransform();
}

void UCWeaponComponent::AttachWeaponToHolster()
{
	if (!IsValid(WeaponActor)) return;

	ClearWeaponPresentationOverride();
	WeaponActor->AttachToHolsterSocket();
	CaptureWeaponAttachmentRelativeTransform();
}

bool UCWeaponComponent::BeginWeaponPresentationOverride(const FTransform& InTargetRelativeOffset, uint32& OutOverrideHandle)
{
	OutOverrideHandle = 0;

	if (!IsValid(WeaponActor)) return false;

	if (ActiveWeaponPresentationOverrideHandle != 0)
	{
		ensureMsgf(
			false,
			TEXT("[Weapon|PresentationOverrideRejected] Reason=AnotherOverrideAlreadyActive | Owner=%s | ActiveHandle=%u"),
			*GetNameSafe(GetOwner()),
			ActiveWeaponPresentationOverrideHandle);
		return false;
	}

	if (!CaptureWeaponAttachmentRelativeTransform()) return false;

	WeaponPresentationTargetRelativeOffset = InTargetRelativeOffset;
	ActiveWeaponPresentationOverrideHandle = NextWeaponPresentationOverrideHandle++;

	// Keep zero as the invalid-handle sentinel even if the counter wraps.
	if (NextWeaponPresentationOverrideHandle == 0)
	{
		++NextWeaponPresentationOverrideHandle;
	}

	if (!ApplyWeaponPresentationOverride(0.f))
	{
		ClearWeaponPresentationOverride();
		return false;
	}

	OutOverrideHandle = ActiveWeaponPresentationOverrideHandle;
	return true;
}

bool UCWeaponComponent::UpdateWeaponPresentationOverride(uint32 InOverrideHandle, float InAlpha)
{
	if (InOverrideHandle == 0 || InOverrideHandle != ActiveWeaponPresentationOverrideHandle) return false;

	return ApplyWeaponPresentationOverride(InAlpha);
}

void UCWeaponComponent::EndWeaponPresentationOverride(uint32 InOverrideHandle)
{
	if (InOverrideHandle == 0 || InOverrideHandle != ActiveWeaponPresentationOverrideHandle) return;

	ClearWeaponPresentationOverride();
}

void UCWeaponComponent::ClearWeaponPresentationOverride()
{
	if (ActiveWeaponPresentationOverrideHandle != 0 && bHasWeaponAttachmentRelativeTransform && IsValid(WeaponActor))
	{
		WeaponActor->SetAttachmentRelativeTransform(WeaponAttachmentRelativeTransform_Base);
	}

	WeaponPresentationTargetRelativeOffset = FTransform::Identity;
	ActiveWeaponPresentationOverrideHandle = 0;
}

void UCWeaponComponent::CommitEquipWeapon()
{
	if (!IsValid(WeaponActor))
	{
		PreserveEquipWeaponTypeWithoutActorForProfiling();
		return;
	}

	ChangeWeaponType(WeaponActor->GetWeaponType());
}

void UCWeaponComponent::CommitUnequipWeapon()
{
	ChangeWeaponType(EWeaponType::Unarmed);
}

void UCWeaponComponent::PushActionDataKey(const FActionDataKey& InActionDataKey)
{
	if (!IsValid(WeaponActor)) return;

	IHitContextProvider* provider = Cast<IHitContextProvider>(WeaponActor);
	if (!provider) return;

	const FWeaponContext weaponContext = BuildWeaponContext();

	provider->SetLastWeaponContext(weaponContext);
	provider->SetLastActionDataKey(InActionDataKey);
}

void UCWeaponComponent::ClearContext()
{
	if (!IsValid(WeaponActor)) return;

	IHitContextProvider* provider = Cast<IHitContextProvider>(WeaponActor);
	if (!provider) return;

	provider->SetLastOverlapContext(FOverlapContext());
	provider->SetLastWeaponContext(FWeaponContext());
	provider->SetLastActionDataKey(FActionDataKey());
}

void UCWeaponComponent::ClearWeaponRuntimeState()
{
	ClearWeaponPresentationOverride();
	ClearContext();

	if (IsValid(WeaponActor))
	{
		WeaponActor->CollisionDisabled();
		WeaponActor->ToggleTrailActive(false);
	}
}

void UCWeaponComponent::StartWeaponDissolve()
{
	if (!IsValid(WeaponActor)) return;

	WeaponActor->ReceiveWeaponDissolveStarted();
}

void UCWeaponComponent::SetWeaponDissolveAmount(float InAmount)
{
	if (!IsValid(WeaponActor)) return;

	WeaponActor->ReceiveWeaponDissolveAmount(InAmount);
}

void UCWeaponComponent::FinishWeaponDissolve()
{
	if (!IsValid(WeaponActor)) return;

	WeaponActor->ReceiveWeaponDissolveFinished();
}

// Weapon Actor

void UCWeaponComponent::DestroyWeaponActor()
{
	if (!IsValid(WeaponActor)) return;

	ClearWeaponPresentationOverride();
	WeaponActor->Destroy();
	WeaponActor = nullptr;
	bHasWeaponAttachmentRelativeTransform = false;
}

void UCWeaponComponent::OpenCollisionWindow(FName InCollisionName)
{
	if (!IsValid(WeaponActor)) return;

	FCombatCollisionProfilingCounters::RecordWeaponComponentOpenCollisionWindow();

	WeaponActor->CollisionEnabled(InCollisionName);
}

void UCWeaponComponent::CloseCollisionWindow()
{
	if (!IsValid(WeaponActor)) return;

	FCombatCollisionProfilingCounters::RecordWeaponComponentCloseCollisionWindow();

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

bool UCWeaponComponent::CaptureWeaponAttachmentRelativeTransform()
{
	if (!IsValid(WeaponActor)) return false;

	FTransform attachmentRelativeTransform;
	if (!WeaponActor->GetAttachmentRelativeTransform(attachmentRelativeTransform)) return false;

	WeaponAttachmentRelativeTransform_Base = attachmentRelativeTransform;
	bHasWeaponAttachmentRelativeTransform = true;
	return true;
}

bool UCWeaponComponent::ApplyWeaponPresentationOverride(float InAlpha)
{
	if (!IsValid(WeaponActor)) return false;
	if (!bHasWeaponAttachmentRelativeTransform) return false;

	const float alpha = FMath::Clamp(InAlpha, 0.f, 1.f);
	const FTransform targetRelativeTransform = WeaponPresentationTargetRelativeOffset * WeaponAttachmentRelativeTransform_Base;

	FTransform blendedRelativeTransform;
	blendedRelativeTransform.Blend(WeaponAttachmentRelativeTransform_Base, targetRelativeTransform, alpha);

	return WeaponActor->SetAttachmentRelativeTransform(blendedRelativeTransform);
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

	if (!ensureMsgf(
		*InWeaponActorClass,
		TEXT("[Weapon|Component|WeaponActorClassMissing] Reason=MissingWeaponActorClass | Owner=%s | Component=%s | Asset=%s | WeaponType=%s"),
		*GetNameSafe(InOwnerCharacter),
		*GetNameSafe(this),
		*GetNameSafe(*InWeaponActorClass),
		*UEnum::GetValueAsString(InWeaponType)))
		return false;

	UWorld* World = InOwnerCharacter->GetWorld();
	if (!World) return false;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = InOwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACWeaponActor* weaponActor = World->SpawnActor<ACWeaponActor>(InWeaponActorClass, SpawnParams);

	if (!ensureMsgf(
		IsValid(weaponActor),
		TEXT("[Weapon|Component|WeaponActorSpawnFailed] Reason=SpawnActorReturnedInvalid | Owner=%s | Component=%s | Asset=%s | WeaponType=%s"),
		*GetNameSafe(InOwnerCharacter),
		*GetNameSafe(this),
		*GetNameSafe(*InWeaponActorClass),
		*UEnum::GetValueAsString(InWeaponType)))
		return false;

	const FCharacterComponentReferences references = BuildWeaponActorReferences();
	weaponActor->InitializeReferences(references);
	weaponActor->ApplyInitialWeaponState(InWeaponType);

	WeaponActor = weaponActor;
	CaptureWeaponAttachmentRelativeTransform();

	return true;
}

// Profiling

bool UCWeaponComponent::ShouldSkipWeaponActorCreationForProfiling() const
{
	return FCombatCollisionProfiling::ShouldSkipEnemyWeaponActorCreation(OwnerCharacter_Injected);
}

void UCWeaponComponent::SkipWeaponActorCreationForProfiling()
{
	bWeaponActorDisabledForProfiling = true;
}

bool UCWeaponComponent::PreserveEquipWeaponTypeWithoutActorForProfiling()
{
	if (!bWeaponActorDisabledForProfiling) return false;
	if (WeaponActorClassKey == EWeaponType::Max) return false;

	ChangeWeaponType(WeaponActorClassKey);
	return true;
}
