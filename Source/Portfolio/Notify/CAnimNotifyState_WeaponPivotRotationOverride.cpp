#include "Notify/CAnimNotifyState_WeaponPivotRotationOverride.h"

#include "Component/CActionComponent.h"
#include "Component/CWeaponComponent.h"

#include "Animation/AnimNotifyLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

FString UCAnimNotifyState_WeaponPivotRotationOverride::GetNotifyName_Implementation() const
{
	return TEXT("Weapon Pivot Rotation Override");
}

void UCAnimNotifyState_WeaponPivotRotationOverride::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
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
		CancelTransition(previousState.WeaponComponent.Get(), previousState.OverrideHandle);
	}

	uint32 overrideHandle = 0;
	if (!BeginTransition(weaponComp, overrideHandle)) return;

	FRuntimeState& runtimeState = RuntimeStates.FindOrAdd(MeshComp);
	runtimeState.WeaponComponent = weaponComp;
	runtimeState.OverrideHandle = overrideHandle;
	runtimeState.ElapsedTime = 0.f;
	runtimeState.TotalDuration = FMath::Max(0.f, TotalDuration);
}

void UCAnimNotifyState_WeaponPivotRotationOverride::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
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

	if (!UpdateTransition(weaponComp, runtimeState->OverrideHandle, CalculateAlpha(runtimeState->ElapsedTime, runtimeState->TotalDuration)))
	{
		CancelTransition(weaponComp, runtimeState->OverrideHandle);
		RuntimeStates.Remove(MeshComp);
	}
}

void UCAnimNotifyState_WeaponPivotRotationOverride::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	FRuntimeState runtimeState;
	if (!RuntimeStates.RemoveAndCopyValue(MeshComp, runtimeState)) return;

	UCWeaponComponent* weaponComp = runtimeState.WeaponComponent.Get();
	if (!IsValid(weaponComp)) return;

	if (!UAnimNotifyLibrary::NotifyStateReachedEnd(EventReference))
	{
		CancelTransition(weaponComp, runtimeState.OverrideHandle);
		return;
	}

	CompleteTransition(weaponComp, runtimeState.OverrideHandle);
}

bool UCAnimNotifyState_WeaponPivotRotationOverride::IsConfigurationValid() const
{
	return BuildRotationSpec().IsValid();
}

bool UCAnimNotifyState_WeaponPivotRotationOverride::BeginTransition(UCWeaponComponent* InWeaponComp, uint32& OutTransitionHandle) const
{
	return IsValid(InWeaponComp)
		&& InWeaponComp->BeginWeaponPivotRotationOverride(BuildRotationSpec(), OutTransitionHandle);
}

bool UCAnimNotifyState_WeaponPivotRotationOverride::UpdateTransition(UCWeaponComponent* InWeaponComp, uint32 InTransitionHandle, float InAlpha) const
{
	return IsValid(InWeaponComp)
		&& InWeaponComp->UpdateWeaponPivotRotationOverride(InTransitionHandle, InAlpha);
}

void UCAnimNotifyState_WeaponPivotRotationOverride::CompleteTransition(UCWeaponComponent* InWeaponComp, uint32 InTransitionHandle) const
{
	if (!IsValid(InWeaponComp) || InTransitionHandle == 0) return;

	InWeaponComp->EndWeaponPivotRotationOverride(InTransitionHandle);
}

void UCAnimNotifyState_WeaponPivotRotationOverride::CancelTransition(UCWeaponComponent* InWeaponComp, uint32 InTransitionHandle) const
{
	if (!IsValid(InWeaponComp) || InTransitionHandle == 0) return;

	InWeaponComp->EndWeaponPivotRotationOverride(InTransitionHandle);
}

FWeaponPivotRotationSpec UCAnimNotifyState_WeaponPivotRotationOverride::BuildRotationSpec() const
{
	FWeaponPivotRotationSpec rotationSpec;
	rotationSpec.Mode = RotationMode;
	rotationSpec.TargetOrientation = RotationOffset;
	rotationSpec.LocalAxis = LocalAxis;
	rotationSpec.SignedAngleDegrees = SignedAngleDegrees;
	return rotationSpec;
}

float UCAnimNotifyState_WeaponPivotRotationOverride::CalculateAlpha(float InElapsedTime, float InTotalDuration) const
{
	if (InTotalDuration <= KINDA_SMALL_NUMBER) return 0.f;

	float blendInDuration = FMath::Max(0.f, BlendInDuration);
	float blendOutDuration = FMath::Max(0.f, BlendOutDuration);
	const float requestedBlendDuration = blendInDuration + blendOutDuration;

	if (requestedBlendDuration > InTotalDuration && requestedBlendDuration > KINDA_SMALL_NUMBER)
	{
		const float scale = InTotalDuration / requestedBlendDuration;
		blendInDuration *= scale;
		blendOutDuration *= scale;
	}

	float alpha = 1.f;
	if (blendInDuration > KINDA_SMALL_NUMBER && InElapsedTime < blendInDuration)
	{
		alpha = InElapsedTime / blendInDuration;
	}
	else
	{
		const float blendOutStartTime = InTotalDuration - blendOutDuration;
		if (blendOutDuration > KINDA_SMALL_NUMBER && InElapsedTime > blendOutStartTime)
		{
			alpha = (InTotalDuration - InElapsedTime) / blendOutDuration;
		}
	}

	return FMath::InterpEaseInOut(
		0.f,
		1.f,
		FMath::Clamp(alpha, 0.f, 1.f),
		FMath::Max(0.01f, BlendExponent));
}
