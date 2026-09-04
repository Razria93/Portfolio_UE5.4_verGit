#include "Notify/CAnimNotifyState_WeaponPresentationOverride.h"

#include "ProjectGlobal.h"

#include "Component/CWeaponComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

UCAnimNotifyState_WeaponPresentationOverride::UCAnimNotifyState_WeaponPresentationOverride()
{
}

FString UCAnimNotifyState_WeaponPresentationOverride::GetNotifyName_Implementation() const
{
	return TEXT("Weapon Presentation Override");
}

void UCAnimNotifyState_WeaponPresentationOverride::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCWeaponComponent* weaponComp = ownerCharacter->FindComponentByClass<UCWeaponComponent>();
	if (!IsValid(weaponComp)) return;

	uint32 overrideHandle = 0;
	if (!weaponComp->BeginWeaponPresentationOverride(TargetRelativeOffset, overrideHandle)) return;

	FRuntimeState& runtimeState = RuntimeStates.FindOrAdd(MeshComp);
	runtimeState.WeaponComponent = weaponComp;
	runtimeState.OverrideHandle = overrideHandle;
	runtimeState.ElapsedTime = 0.f;
	runtimeState.TotalDuration = FMath::Max(0.f, TotalDuration);
}

void UCAnimNotifyState_WeaponPresentationOverride::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!IsValid(MeshComp)) return;

	FRuntimeState* runtimeState = RuntimeStates.Find(MeshComp);
	if (!runtimeState) return;

	UCWeaponComponent* weaponComp = runtimeState->WeaponComponent.Get();
	if (!IsValid(weaponComp))
	{
		RuntimeStates.Remove(MeshComp);
		return;
	}

	runtimeState->ElapsedTime = FMath::Clamp(runtimeState->ElapsedTime + FMath::Max(0.f, FrameDeltaTime), 0.f, runtimeState->TotalDuration);
	const float alpha = CalculatePresentationAlpha(runtimeState->ElapsedTime, runtimeState->TotalDuration);

	weaponComp->UpdateWeaponPresentationOverride(runtimeState->OverrideHandle, alpha);
}

void UCAnimNotifyState_WeaponPresentationOverride::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	FRuntimeState runtimeState;
	if (!RuntimeStates.RemoveAndCopyValue(MeshComp, runtimeState)) return;

	if (UCWeaponComponent* weaponComp = runtimeState.WeaponComponent.Get())
	{
		weaponComp->EndWeaponPresentationOverride(runtimeState.OverrideHandle);
	}
}

float UCAnimNotifyState_WeaponPresentationOverride::CalculatePresentationAlpha(float InElapsedTime, float InTotalDuration) const
{
	if (InTotalDuration <= KINDA_SMALL_NUMBER) return 0.f;

	float blendInDuration = FMath::Max(0.f, BlendInDuration);
	float blendOutDuration = FMath::Max(0.f, BlendOutDuration);
	const float requestedBlendDuration = blendInDuration + blendOutDuration;

	// Preserve both blend directions when a short notify window cannot contain the
	// requested in/out durations. This avoids a discontinuity at either endpoint.
	if (requestedBlendDuration > InTotalDuration && requestedBlendDuration > KINDA_SMALL_NUMBER)
	{
		const float scale = InTotalDuration / requestedBlendDuration;
		blendInDuration *= scale;
		blendOutDuration *= scale;
	}

	if (blendInDuration > KINDA_SMALL_NUMBER && InElapsedTime < blendInDuration)
	{
		return EaseInOut(InElapsedTime / blendInDuration);
	}

	const float blendOutStartTime = InTotalDuration - blendOutDuration;
	if (blendOutDuration > KINDA_SMALL_NUMBER && InElapsedTime > blendOutStartTime)
	{
		return EaseInOut((InTotalDuration - InElapsedTime) / blendOutDuration);
	}

	return 1.f;
}

float UCAnimNotifyState_WeaponPresentationOverride::EaseInOut(float InAlpha)
{
	return FMath::InterpEaseInOut(0.f, 1.f, FMath::Clamp(InAlpha, 0.f, 1.f), 2.f);
}
