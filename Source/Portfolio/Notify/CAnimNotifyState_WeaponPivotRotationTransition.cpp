#include "Notify/CAnimNotifyState_WeaponPivotRotationTransition.h"

#include "Component/CActionComponent.h"
#include "Component/CWeaponComponent.h"

#include "Animation/AnimNotifyLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

FString UCAnimNotifyState_WeaponPivotRotationTransition::GetNotifyName_Implementation() const
{
	return TEXT("Weapon Pivot Rotation Transition");
}

void UCAnimNotifyState_WeaponPivotRotationTransition::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!IsValid(MeshComp) || !IsConfigurationValid()) return;

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCWeaponComponent* weaponComp = ownerCharacter->FindComponentByClass<UCWeaponComponent>();
	if (!IsValid(weaponComp)) return;

	FRuntimeState previousState;
	if (RuntimeStates.RemoveAndCopyValue(MeshComp, previousState))
	{
		CancelTransition(previousState.WeaponComponent.Get(), previousState.TransitionHandle);
	}

	uint32 transitionHandle = 0;
	if (!BeginTransition(weaponComp, transitionHandle)) return;

	FRuntimeState& runtimeState = RuntimeStates.FindOrAdd(MeshComp);
	runtimeState.WeaponComponent = weaponComp;
	runtimeState.TransitionHandle = transitionHandle;
	runtimeState.ElapsedTime = 0.f;
	runtimeState.TotalDuration = FMath::Max(0.f, TotalDuration);
}

void UCAnimNotifyState_WeaponPivotRotationTransition::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	FRuntimeState* runtimeState = RuntimeStates.Find(MeshComp);
	if (!runtimeState) return;

	UCWeaponComponent* weaponComp = runtimeState->WeaponComponent.Get();
	if (!IsValid(weaponComp))
	{
		RuntimeStates.Remove(MeshComp);
		return;
	}

	runtimeState->ElapsedTime = FMath::Clamp(
		runtimeState->ElapsedTime + FMath::Max(0.f, FrameDeltaTime),
		0.f,
		runtimeState->TotalDuration);

	if (!UpdateTransition(weaponComp, runtimeState->TransitionHandle, CalculateAlpha(runtimeState->ElapsedTime, runtimeState->TotalDuration)))
	{
		CancelTransition(weaponComp, runtimeState->TransitionHandle);
		RuntimeStates.Remove(MeshComp);
	}
}

void UCAnimNotifyState_WeaponPivotRotationTransition::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	FRuntimeState runtimeState;
	if (!RuntimeStates.RemoveAndCopyValue(MeshComp, runtimeState)) return;

	UCWeaponComponent* weaponComp = runtimeState.WeaponComponent.Get();
	if (!IsValid(weaponComp)) return;

	if (!UAnimNotifyLibrary::NotifyStateReachedEnd(EventReference))
	{
		CancelTransition(weaponComp, runtimeState.TransitionHandle);
		return;
	}

	CompleteTransition(weaponComp, runtimeState.TransitionHandle);
}

bool UCAnimNotifyState_WeaponPivotRotationTransition::IsConfigurationValid() const
{
	return BuildRotationSpec().IsValid();
}

bool UCAnimNotifyState_WeaponPivotRotationTransition::BeginTransition(UCWeaponComponent* InWeaponComp, uint32& OutTransitionHandle) const
{
	return IsValid(InWeaponComp)
		&& InWeaponComp->BeginWeaponPivotRotationTransition(BuildRotationSpec(), OutTransitionHandle);
}

bool UCAnimNotifyState_WeaponPivotRotationTransition::UpdateTransition(UCWeaponComponent* InWeaponComp, uint32 InTransitionHandle, float InAlpha) const
{
	return IsValid(InWeaponComp)
		&& InWeaponComp->UpdateWeaponPivotRotationTransition(InTransitionHandle, InAlpha);
}

bool UCAnimNotifyState_WeaponPivotRotationTransition::CompleteTransition(UCWeaponComponent* InWeaponComp, uint32 InTransitionHandle) const
{
	return IsValid(InWeaponComp)
		&& InWeaponComp->CompleteWeaponPivotRotationTransition(InTransitionHandle);
}

void UCAnimNotifyState_WeaponPivotRotationTransition::CancelTransition(UCWeaponComponent* InWeaponComp, uint32 InTransitionHandle) const
{
	if (!IsValid(InWeaponComp) || InTransitionHandle == 0) return;

	InWeaponComp->CancelWeaponPivotRotationTransition(InTransitionHandle);
}

FWeaponPivotRotationSpec UCAnimNotifyState_WeaponPivotRotationTransition::BuildRotationSpec() const
{
	FWeaponPivotRotationSpec rotationSpec;
	rotationSpec.Mode = RotationMode;
	rotationSpec.TargetOrientation = TargetRotation;
	rotationSpec.LocalAxis = LocalAxis;
	rotationSpec.SignedAngleDegrees = SignedAngleDegrees;
	return rotationSpec;
}

float UCAnimNotifyState_WeaponPivotRotationTransition::CalculateAlpha(float InElapsedTime, float InTotalDuration) const
{
	if (InTotalDuration <= KINDA_SMALL_NUMBER) return 1.f;

	return FMath::InterpEaseInOut(
		0.f,
		1.f,
		FMath::Clamp(InElapsedTime / InTotalDuration, 0.f, 1.f),
		FMath::Max(0.01f, BlendExponent));
}
