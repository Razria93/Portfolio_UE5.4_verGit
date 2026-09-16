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

// Query

ACWeaponActor* UCWeaponComponent::GetWeaponActor() const
{
	return IsValid(WeaponActor) ? WeaponActor : nullptr;
}

// Weapon Presentation - Action Pose Scope

bool UCWeaponComponent::BeginWeaponActionPoseScope(uint32 InScopeHandle)
{
	if (InScopeHandle == 0 || ActionPoseScopeState.ActiveHandle != 0) return false;

	ClearWeaponSocketTransformTransition(true);
	ClearWeaponPivotRuntimeEffects();

	ActionPoseScopeState.ActiveHandle = InScopeHandle;

	ActionPoseScopeState.ScopedWeaponActor = WeaponActor;

	ActionPoseScopeState.bHasSocketBaseline = false;

	ActionPoseScopeState.BaselineSocketName = NAME_None;
	ActionPoseScopeState.BaselinePivotRotation = CommittedPivotRotation;

	if (IsValid(WeaponActor))
	{
		ActionPoseScopeState.bHasSocketBaseline = WeaponActor->GetCurrentOwnerSocketName(ActionPoseScopeState.BaselineSocketName);
	}

	return true;
}

bool UCWeaponComponent::EndWeaponActionPoseScope(uint32 InScopeHandle)
{
	if (!IsWeaponActionPoseScopeActive(InScopeHandle)) return false;

	ACWeaponActor* scopedWeaponActor = ActionPoseScopeState.ScopedWeaponActor.Get();

	const bool bCanRestoreScopedActor = IsValid(scopedWeaponActor) && scopedWeaponActor == WeaponActor;
	const bool bRestoreSocket = bCanRestoreScopedActor && ActionPoseScopeState.bHasSocketBaseline;

	const FName baselineSocketName = ActionPoseScopeState.BaselineSocketName;
	const FQuat baselinePivotRotation = ActionPoseScopeState.BaselinePivotRotation;

	// Invalidate first so late NotifyEnd callbacks cannot mutate the next action.
	ActionPoseScopeState.ActiveHandle = 0;
	ClearWeaponSocketTransformTransition(false);
	ClearWeaponPivotRuntimeEffects();

	if (bCanRestoreScopedActor)
	{
		if (bRestoreSocket)
		{
			WeaponActor->AttachToOwnerSocket(baselineSocketName);
		}

		CommittedPivotRotation = baselinePivotRotation;
		ApplyWeaponPivotRotation();
	}

	ResetWeaponActionPoseScope();
	return true;
}

bool UCWeaponComponent::IsWeaponActionPoseScopeActive(uint32 InScopeHandle) const
{
	return InScopeHandle != 0 && InScopeHandle == ActionPoseScopeState.ActiveHandle;
}

// Weapon Presentation - Socket Transform Transition

bool UCWeaponComponent::BeginWeaponSocketTransformTransition(EWeaponSocketSlot InTargetSlot, uint32& OutTransitionHandle)
{
	OutTransitionHandle = 0;

	if (ActionPoseScopeState.ActiveHandle == 0) return false;
	if (!IsValid(WeaponActor)) return false;
	if (InTargetSlot == EWeaponSocketSlot::None || InTargetSlot == EWeaponSocketSlot::Max) return false;

	FName targetSocketName;
	if (!WeaponActor->ResolveSocketName(InTargetSlot, targetSocketName)) return false;

	return BeginWeaponSocketTransformTransitionInternal(ActionPoseScopeState.ActiveHandle, NAME_None, targetSocketName, OutTransitionHandle);
}

bool UCWeaponComponent::BeginWeaponSocketTransformTransition(FName InExpectedSourceSocketName, FName InTargetSocketName, uint32& OutTransitionHandle)
{
	OutTransitionHandle = 0;

	if (ActionPoseScopeState.ActiveHandle == 0) return false;

	return BeginWeaponSocketTransformTransitionInternal(ActionPoseScopeState.ActiveHandle, InExpectedSourceSocketName, InTargetSocketName, OutTransitionHandle);
}

bool UCWeaponComponent::UpdateWeaponSocketTransformTransition(uint32 InTransitionHandle, float InAlpha)
{
	if (InTransitionHandle == 0 || InTransitionHandle != SocketTransformTransitionState.ActiveHandle) return false;
	if (!IsWeaponActionPoseScopeActive(SocketTransformTransitionState.ActionPoseScopeHandle)) return false;
	if (!IsValid(WeaponActor)) return false;

	FTransform sourceWorldTransform;
	if (!WeaponActor->GetOwnerSocketWorldTransform(SocketTransformTransitionState.SourceSocketName, sourceWorldTransform))
	{
		return false;
	}

	FTransform targetWorldTransform;
	if (!WeaponActor->GetOwnerSocketWorldTransform(SocketTransformTransitionState.TargetSocketName, targetWorldTransform)) return false;

	FTransform blendedWorldTransform;
	blendedWorldTransform.Blend(sourceWorldTransform, targetWorldTransform, FMath::Clamp(InAlpha, 0.f, 1.f));

	return WeaponActor->SetActorRootWorldTransform(blendedWorldTransform);
}

bool UCWeaponComponent::CompleteWeaponSocketTransformTransition(uint32 InTransitionHandle)
{
	if (InTransitionHandle == 0 || InTransitionHandle != SocketTransformTransitionState.ActiveHandle) return false;
	if (!IsWeaponActionPoseScopeActive(SocketTransformTransitionState.ActionPoseScopeHandle)) return false;
	if (!IsValid(WeaponActor)) return false;

	const bool bAttached = WeaponActor->AttachToOwnerSocket(SocketTransformTransitionState.TargetSocketName);
	ClearWeaponSocketTransformTransition(false);
	return bAttached;
}

void UCWeaponComponent::CancelWeaponSocketTransformTransition(uint32 InTransitionHandle)
{
	if (InTransitionHandle == 0 || InTransitionHandle != SocketTransformTransitionState.ActiveHandle) return;
	if (!IsWeaponActionPoseScopeActive(SocketTransformTransitionState.ActionPoseScopeHandle)) return;

	ClearWeaponSocketTransformTransition(true);
}

// Weapon Presentation - Pivot Rotation Transition

bool UCWeaponComponent::BeginWeaponPivotRotationTransition(const FWeaponPivotRotationSpec& InRotationSpec, uint32& OutTransitionHandle)
{
	OutTransitionHandle = 0;

	if (ActionPoseScopeState.ActiveHandle == 0 || !InRotationSpec.IsValid()) return false;
	if (!IsValid(WeaponActor) || !WeaponActor->HasValidPivot()) return false;

	if (PivotRotationTransitionState.ActiveHandle != 0)
	{
		ensureMsgf(false,
			TEXT("[Weapon|PivotRotationTransitionRejected] Reason=AnotherTransitionAlreadyActive | Owner=%s | ActiveHandle=%u"),
			*GetNameSafe(GetOwner()),
			PivotRotationTransitionState.ActiveHandle);
		return false;
	}

	PivotRotationTransitionState.SourceRotation = CommittedPivotRotation;
	PivotRotationTransitionState.CurrentRotation = CommittedPivotRotation;
	PivotRotationTransitionState.RotationSpec = InRotationSpec;
	PivotRotationTransitionState.ActiveHandle = PivotRotationTransitionState.NextHandle++;
	PivotRotationTransitionState.ActionPoseScopeHandle = ActionPoseScopeState.ActiveHandle;

	if (PivotRotationTransitionState.NextHandle == 0)
	{
		++PivotRotationTransitionState.NextHandle;
	}

	if (!ApplyWeaponPivotRotation())
	{
		ClearWeaponPivotRotationTransitionState();
		return false;
	}

	OutTransitionHandle = PivotRotationTransitionState.ActiveHandle;
	return true;
}

bool UCWeaponComponent::UpdateWeaponPivotRotationTransition(uint32 InTransitionHandle, float InAlpha)
{
	if (InTransitionHandle == 0 || InTransitionHandle != PivotRotationTransitionState.ActiveHandle) return false;
	if (PivotRotationTransitionState.ActionPoseScopeHandle == 0 || !IsWeaponActionPoseScopeActive(PivotRotationTransitionState.ActionPoseScopeHandle)) return false;

	PivotRotationTransitionState.CurrentRotation = PivotRotationTransitionState.RotationSpec.Evaluate(PivotRotationTransitionState.SourceRotation, InAlpha);
	return ApplyWeaponPivotRotation();
}

bool UCWeaponComponent::CompleteWeaponPivotRotationTransition(uint32 InTransitionHandle)
{
	if (InTransitionHandle == 0 || InTransitionHandle != PivotRotationTransitionState.ActiveHandle) return false;
	if (PivotRotationTransitionState.ActionPoseScopeHandle == 0 || !IsWeaponActionPoseScopeActive(PivotRotationTransitionState.ActionPoseScopeHandle)) return false;

	CommittedPivotRotation = PivotRotationTransitionState.RotationSpec.Evaluate(PivotRotationTransitionState.SourceRotation, 1.f);
	ClearWeaponPivotRotationTransitionState();
	return ApplyWeaponPivotRotation();
}

void UCWeaponComponent::CancelWeaponPivotRotationTransition(uint32 InTransitionHandle)
{
	if (InTransitionHandle == 0 || InTransitionHandle != PivotRotationTransitionState.ActiveHandle) return;
	if (PivotRotationTransitionState.ActionPoseScopeHandle == 0 || !IsWeaponActionPoseScopeActive(PivotRotationTransitionState.ActionPoseScopeHandle)) return;

	ClearWeaponPivotRotationTransitionState();
	ApplyWeaponPivotRotation();
}

// Weapon Presentation - Pivot Rotation Override

bool UCWeaponComponent::BeginWeaponPivotRotationOverride(const FWeaponPivotRotationSpec& InRotationSpec, uint32& OutOverrideHandle)
{
	OutOverrideHandle = 0;

	if (ActionPoseScopeState.ActiveHandle == 0 || !InRotationSpec.IsValid()) return false;
	if (!IsValid(WeaponActor) || !WeaponActor->HasValidPivot()) return false;

	if (PivotRotationOverrideState.ActiveHandle != 0)
	{
		ensureMsgf(false,
			TEXT("[Weapon|PivotRotationOverrideRejected] Reason=AnotherOverrideAlreadyActive | Owner=%s | ActiveHandle=%u"),
			*GetNameSafe(GetOwner()),
			PivotRotationOverrideState.ActiveHandle);
		return false;
	}

	PivotRotationOverrideState.TemporaryRotationOffset = FQuat::Identity;
	PivotRotationOverrideState.RotationSpec = InRotationSpec;
	PivotRotationOverrideState.ActiveHandle = PivotRotationOverrideState.NextHandle++;
	PivotRotationOverrideState.ActionPoseScopeHandle = ActionPoseScopeState.ActiveHandle;

	if (PivotRotationOverrideState.NextHandle == 0)
	{
		++PivotRotationOverrideState.NextHandle;
	}

	if (!ApplyWeaponPivotRotation())
	{
		ClearWeaponPivotRotationOverrideState();
		return false;
	}

	OutOverrideHandle = PivotRotationOverrideState.ActiveHandle;
	return true;
}

bool UCWeaponComponent::UpdateWeaponPivotRotationOverride(uint32 InOverrideHandle, float InAlpha)
{
	if (InOverrideHandle == 0 || InOverrideHandle != PivotRotationOverrideState.ActiveHandle) return false;
	if (PivotRotationOverrideState.ActionPoseScopeHandle == 0 || !IsWeaponActionPoseScopeActive(PivotRotationOverrideState.ActionPoseScopeHandle)) return false;

	PivotRotationOverrideState.TemporaryRotationOffset = PivotRotationOverrideState.RotationSpec.Evaluate(FQuat::Identity, InAlpha);
	return ApplyWeaponPivotRotation();
}

void UCWeaponComponent::EndWeaponPivotRotationOverride(uint32 InOverrideHandle)
{
	if (InOverrideHandle == 0 || InOverrideHandle != PivotRotationOverrideState.ActiveHandle) return;
	if (PivotRotationOverrideState.ActionPoseScopeHandle == 0 || !IsWeaponActionPoseScopeActive(PivotRotationOverrideState.ActionPoseScopeHandle)) return;

	ClearWeaponPivotRotationOverrideState();
	ApplyWeaponPivotRotation();
}

// Legacy Weapon Presentation Override compatibility

bool UCWeaponComponent::BeginWeaponPresentationOverride(const FTransform& InTargetRelativeOffset, uint32& OutOverrideHandle)
{
	if (InTargetRelativeOffset.ContainsNaN())
	{
		OutOverrideHandle = 0;
		return false;
	}

	FWeaponPivotRotationSpec rotationSpec;
	rotationSpec.Mode = EWeaponPivotRotationMode::TargetOrientation;
	rotationSpec.TargetOrientation = InTargetRelativeOffset.GetRotation().Rotator();
	return BeginWeaponPivotRotationOverride(rotationSpec, OutOverrideHandle);
}

bool UCWeaponComponent::UpdateWeaponPresentationOverride(uint32 InOverrideHandle, float InAlpha)
{
	return UpdateWeaponPivotRotationOverride(InOverrideHandle, InAlpha);
}

void UCWeaponComponent::EndWeaponPresentationOverride(uint32 InOverrideHandle)
{
	EndWeaponPivotRotationOverride(InOverrideHandle);
}

// Equip / Unequip Commit

void UCWeaponComponent::CommitEquipWeapon()
{
	if (!IsValid(WeaponActor))
	{
		PreserveEquipWeaponTypeWithoutActorForProfiling();
		return;
	}

	ChangeWeaponType(WeaponActor->GetWeaponType());
	RebaseWeaponActionPoseScopeSocket();
}

void UCWeaponComponent::CommitUnequipWeapon()
{
	ChangeWeaponType(EWeaponType::Unarmed);
	RebaseWeaponActionPoseScopeSocket();
}

// Combat Context

void UCWeaponComponent::PushActionDataKey(const FActionDataKey& InActionDataKey)
{
	if (!IsValid(WeaponActor)) return;

	IHitContextProvider* provider = Cast<IHitContextProvider>(WeaponActor);
	if (!provider) return;

	const FWeaponContext weaponContext = BuildWeaponContext();

	provider->SetLastWeaponContext(weaponContext);
	provider->SetLastActionDataKey(InActionDataKey);
}

void UCWeaponComponent::ClearWeaponCombatContext()
{
	if (!IsValid(WeaponActor)) return;

	IHitContextProvider* provider = Cast<IHitContextProvider>(WeaponActor);
	if (!provider) return;

	provider->SetLastOverlapContext(FOverlapContext());
	provider->SetLastWeaponContext(FWeaponContext());
	provider->SetLastActionDataKey(FActionDataKey());
}

// Collision

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

// Feedback - Dissolve

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

// Runtime Cleanup

void UCWeaponComponent::ClearWeaponRuntimeState()
{
	ClearWeaponSocketTransformTransition(true);
	ClearWeaponPivotRuntimeEffects();
	ClearWeaponCombatContext();

	if (IsValid(WeaponActor))
	{
		WeaponActor->CollisionDisabled();
		WeaponActor->DeactivateAllTrails();
	}
}

// Weapon Runtime Lifecycle

void UCWeaponComponent::UninitializeWeaponRuntime()
{
	ClearWeaponRuntimeState();
	DestroyWeaponActor();
}

// Weapon Actor Lifecycle

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
	ResetWeaponPivotRotationState();

	return true;
}

void UCWeaponComponent::DestroyWeaponActor()
{
	if (!IsValid(WeaponActor))
	{
		ResetWeaponActionPoseScope();
		return;
	}

	ClearWeaponSocketTransformTransition(false);
	ResetWeaponPivotRotationState();
	ResetWeaponActionPoseScope();
	WeaponActor->Destroy();
	WeaponActor = nullptr;
}

// Weapon State Helpers

void UCWeaponComponent::ChangeWeaponType(EWeaponType InNewWeaponType)
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	EWeaponType prevWeaponType = CurrentWeaponType;
	CurrentWeaponType = InNewWeaponType;

	if (OnWeaponTypeChanged.IsBound())
		OnWeaponTypeChanged.Broadcast(OwnerCharacter_Injected, prevWeaponType, CurrentWeaponType);
}

// Weapon Presentation - Action Pose Scope Helpers

void UCWeaponComponent::RebaseWeaponActionPoseScopeSocket()
{
	if (ActionPoseScopeState.ActiveHandle == 0 || !IsValid(WeaponActor)) return;
	if (ActionPoseScopeState.ScopedWeaponActor.Get() != WeaponActor) return;

	ActionPoseScopeState.bHasSocketBaseline = WeaponActor->GetCurrentOwnerSocketName(ActionPoseScopeState.BaselineSocketName);
}

void UCWeaponComponent::ResetWeaponActionPoseScope()
{
	ActionPoseScopeState = FWeaponActionPoseScopeState();
	SocketTransformTransitionState.ActionPoseScopeHandle = 0;
	PivotRotationTransitionState.ActionPoseScopeHandle = 0;
	PivotRotationOverrideState.ActionPoseScopeHandle = 0;
}

// Weapon Presentation - Socket Transform Transition Helpers

bool UCWeaponComponent::BeginWeaponSocketTransformTransitionInternal(uint32 InActionPoseScopeHandle, FName InExpectedSourceSocketName, FName InTargetSocketName, uint32& OutTransitionHandle)
{
	OutTransitionHandle = 0;

	if (!IsValid(WeaponActor) || InTargetSocketName.IsNone()) return false;

	if (SocketTransformTransitionState.ActiveHandle != 0)
	{
		ensureMsgf(false,
			TEXT("[Weapon|SocketTransformTransitionRejected] Reason=AnotherTransitionAlreadyActive | Owner=%s | ActiveHandle=%u"),
			*GetNameSafe(GetOwner()),
			SocketTransformTransitionState.ActiveHandle);
		return false;
	}

	FName sourceSocketName;
	FTransform unusedTargetWorldTransform;
	if (!WeaponActor->GetCurrentOwnerSocketName(sourceSocketName)) return false;
	if (!InExpectedSourceSocketName.IsNone() && sourceSocketName != InExpectedSourceSocketName) return false;
	if (!WeaponActor->GetOwnerSocketWorldTransform(InTargetSocketName, unusedTargetWorldTransform)) return false;

	SocketTransformTransitionState.SourceSocketName = sourceSocketName;
	SocketTransformTransitionState.TargetSocketName = InTargetSocketName;
	SocketTransformTransitionState.ActionPoseScopeHandle = InActionPoseScopeHandle;
	SocketTransformTransitionState.ActiveHandle = SocketTransformTransitionState.NextHandle++;

	if (SocketTransformTransitionState.NextHandle == 0)
	{
		++SocketTransformTransitionState.NextHandle;
	}

	OutTransitionHandle = SocketTransformTransitionState.ActiveHandle;
	return true;
}

bool UCWeaponComponent::ClearWeaponSocketTransformTransition(bool bRestoreSourceSocket)
{
	if (SocketTransformTransitionState.ActiveHandle == 0)
	{
		return false;
	}

	if (bRestoreSourceSocket && IsValid(WeaponActor))
	{
		WeaponActor->AttachToOwnerSocket(SocketTransformTransitionState.SourceSocketName);
	}

	SocketTransformTransitionState.SourceSocketName = NAME_None;
	SocketTransformTransitionState.TargetSocketName = NAME_None;
	SocketTransformTransitionState.ActiveHandle = 0;
	SocketTransformTransitionState.ActionPoseScopeHandle = 0;
	return true;
}

// Weapon Presentation - Pivot Helpers

bool UCWeaponComponent::ApplyWeaponPivotRotation()
{
	if (!IsValid(WeaponActor)) return false;

	const FQuat baseRotation = PivotRotationTransitionState.ActiveHandle != 0
		? PivotRotationTransitionState.CurrentRotation
		: CommittedPivotRotation;

	FQuat finalRotation = baseRotation * PivotRotationOverrideState.TemporaryRotationOffset;
	finalRotation.Normalize();

	return WeaponActor->ApplyPivotRotation(finalRotation);
}

void UCWeaponComponent::ClearWeaponPivotRotationTransitionState()
{
	PivotRotationTransitionState.SourceRotation = FQuat::Identity;
	PivotRotationTransitionState.CurrentRotation = FQuat::Identity;
	PivotRotationTransitionState.RotationSpec = FWeaponPivotRotationSpec();
	PivotRotationTransitionState.ActiveHandle = 0;
	PivotRotationTransitionState.ActionPoseScopeHandle = 0;
}

void UCWeaponComponent::ClearWeaponPivotRotationOverrideState()
{
	PivotRotationOverrideState.TemporaryRotationOffset = FQuat::Identity;
	PivotRotationOverrideState.RotationSpec = FWeaponPivotRotationSpec();
	PivotRotationOverrideState.ActiveHandle = 0;
	PivotRotationOverrideState.ActionPoseScopeHandle = 0;
}

void UCWeaponComponent::ClearWeaponPivotRuntimeEffects()
{
	ClearWeaponPivotRotationTransitionState();
	ClearWeaponPivotRotationOverrideState();
	ApplyWeaponPivotRotation();
}

void UCWeaponComponent::ResetWeaponPivotRotationState()
{
	const uint32 nextTransitionHandle = PivotRotationTransitionState.NextHandle;
	const uint32 nextOverrideHandle = PivotRotationOverrideState.NextHandle;

	CommittedPivotRotation = FQuat::Identity;
	PivotRotationTransitionState = FWeaponPivotRotationTransitionState();
	PivotRotationOverrideState = FWeaponPivotRotationOverrideState();
	PivotRotationTransitionState.NextHandle = nextTransitionHandle;
	PivotRotationOverrideState.NextHandle = nextOverrideHandle;

	if (IsValid(WeaponActor))
	{
		WeaponActor->ResetPivotRotation();
	}
}

// Combat Context Helpers

FWeaponContext UCWeaponComponent::BuildWeaponContext() const
{
	FWeaponContext weaponContext;

	weaponContext.WeaponType = CurrentWeaponType;

	return weaponContext;
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
