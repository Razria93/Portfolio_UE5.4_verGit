#include "Component/CActionFeedbackComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

UCActionFeedbackComponent::UCActionFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCActionFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor_Cached = GetOwner();
	check(OwnerActor_Cached);

	OwnerCharacter_Cached = Cast<ACharacter>(OwnerActor_Cached);
	check(OwnerCharacter_Cached);
}

void UCActionFeedbackComponent::PlayActionFeedback(const FHitContext& InHitContext, EActionFeedbackPhase InActionFeedbackPhase)
{
	if (!CanPlayActionFeedback(InHitContext, InActionFeedbackPhase)) return;

	switch (InActionFeedbackPhase)
	{
	case EActionFeedbackPhase::ActionStart:
		PlayActionStartVFX(InHitContext);
		PlayActionStartSound(InHitContext);
		break;

	case EActionFeedbackPhase::AttackWindowBegin:
		PlayAttackWindowBeginFeedback(InHitContext);
		break;

	case EActionFeedbackPhase::AttackWindowEnd:
		PlayAttackWindowEndFeedback(InHitContext);
		break;

	case EActionFeedbackPhase::ActionEnd:
		SetWeaponTrailEnabled(false);
		break;

	default:
		break;
	}
}

void UCActionFeedbackComponent::PlayActionStartVFX(const FHitContext& InHitContext)
{
	if (!IsValid(ActionStartVFX)) return;

	UNiagaraFunctionLibrary::SpawnSystemAttached(
		ActionStartVFX,							// NiagaraSystem
		OwnerCharacter_Cached->GetMesh(),		// AttachComponent
		NAME_None,								// SocketName
		FVector::ZeroVector,					// Location
		FRotator::ZeroRotator,					// Rotation
		EAttachLocation::KeepRelativeOffset,	// LocationType
		true									// bAutoDestroy
	);
}

void UCActionFeedbackComponent::PlayActionStartSound(const FHitContext& InHitContext)
{
	if (!IsValid(ActionStartSound)) return;

	UGameplayStatics::PlaySoundAtLocation(this, ActionStartSound, OwnerActor_Cached->GetActorLocation());
}

void UCActionFeedbackComponent::PlayAttackWindowBeginFeedback(const FHitContext& InHitContext)
{
	SetWeaponTrailEnabled(true);
}

void UCActionFeedbackComponent::PlayAttackWindowEndFeedback(const FHitContext& InHitContext)
{
	SetWeaponTrailEnabled(false);
}

bool UCActionFeedbackComponent::CanPlayActionFeedback(const FHitContext& InHitContext, EActionFeedbackPhase InActionFeedbackPhase) const
{
	if (!IsValid(OwnerActor_Cached)) return false;
	if (!IsValid(GetWorld())) return false;
	if (InActionFeedbackPhase == EActionFeedbackPhase::None) return false;

	return true;
}

FApplyDamageSpecKey UCActionFeedbackComponent::BuildActionFeedbackSpecKey(const FHitContext& InHitContext) const
{
	FApplyDamageSpecKey applyDamageSpecKey;

	applyDamageSpecKey.AttachmentType = InHitContext.AttachmentContext.CurrentAttachmentType;
	applyDamageSpecKey.EquipmentType = InHitContext.EquipmentContext.CurrentEquipmentType;
	applyDamageSpecKey.ActionType = InHitContext.ActionContext.CurrentActionType;
	applyDamageSpecKey.ActionIndex = InHitContext.ActionContext.ActionIndex;

	return applyDamageSpecKey;
}

void UCActionFeedbackComponent::SetWeaponTrailEnabled(bool bEnable)
{
	if (!bUseWeaponTrail) return;

	// TODO:
	// Attachment/Equipment trail toggle
}
