#include "Component/CActionFeedbackComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

#include "Component/CWeaponComponent.h"
#include "Weapon/CAttachment.h"

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

	WeaponComp_Cached = Cast<UCWeaponComponent>(OwnerActor_Cached->GetComponentByClass(UCWeaponComponent::StaticClass()));
	check(WeaponComp_Cached);
}

void UCActionFeedbackComponent::PlayActionFeedback(const FApplyDamageSpecKey& InApplyDamageSpecKey, EActionFeedbackPhase InActionFeedbackPhase)
{
	if (!CanPlayActionFeedback(InActionFeedbackPhase)) return;

	// Debug
	PrintActionFeedbackRequestInfo(InApplyDamageSpecKey, InActionFeedbackPhase);

	FActionFeedbackData actionFeedbackData;
	if (!ResolveActionFeedbackData(InActionFeedbackPhase, actionFeedbackData))
	{
		FLog::Log(FString::Printf(TEXT("[PlayActionFeedback] Failed to resolve data. Phase = %s"), *UEnum::GetValueAsString(InActionFeedbackPhase)));
		return;
	}

	// Debug
	PrintActionFeedbackDataInfo(actionFeedbackData);

	ExecuteActionFeedback(actionFeedbackData);
}

void UCActionFeedbackComponent::ExecuteActionFeedback(const FActionFeedbackData& InActionFeedbackData)
{
	PlayActionVFX(InActionFeedbackData.ActionVFX);
	PlayActionSFX(InActionFeedbackData.ActionSFX);
	SetActionTrailActive(InActionFeedbackData.bTrailActive);
}

void UCActionFeedbackComponent::PlayActionVFX(UNiagaraSystem* InActionVFX)
{
	if (!IsValid(InActionVFX)) return;
	if (!IsValid(OwnerCharacter_Cached)) return;

	UNiagaraFunctionLibrary::SpawnSystemAttached(
		InActionVFX,							// NiagaraSystem
		OwnerCharacter_Cached->GetMesh(),		// AttachComponent
		NAME_None,								// SocketName
		FVector::ZeroVector,					// Location
		FRotator::ZeroRotator,					// Rotation
		EAttachLocation::KeepRelativeOffset,	// LocationType
		true									// bAutoDestroy
	);
}

void UCActionFeedbackComponent::PlayActionSFX(USoundBase* InActionSFX)
{
	if (!IsValid(InActionSFX)) return;
	if (!IsValid(OwnerActor_Cached)) return;

	UGameplayStatics::PlaySoundAtLocation(this, InActionSFX, OwnerActor_Cached->GetActorLocation());
}

void UCActionFeedbackComponent::SetActionTrailActive(bool bActive)
{
	if (!IsValid(WeaponComp_Cached)) return;

	UObject* uobject = WeaponComp_Cached->GetAttachment();
	if (!IsValid(uobject)) return;

	ACAttachment* attachment = Cast<ACAttachment>(uobject);
	if (!IsValid(attachment)) return;

	// Debug
	PrintActionTrailInfo(bActive, attachment);
	
	attachment->SetActionTrailActive(bActive);
}

bool UCActionFeedbackComponent::CanPlayActionFeedback(EActionFeedbackPhase InActionFeedbackPhase) const
{
	if (!IsValid(OwnerActor_Cached)) return false;
	if (!IsValid(GetWorld())) return false;
	if (InActionFeedbackPhase == EActionFeedbackPhase::None) return false;

	return true;
}

bool UCActionFeedbackComponent::ResolveActionFeedbackData(EActionFeedbackPhase InActionFeedbackPhase, FActionFeedbackData& OutActionFeedbackData) const
{
	switch (InActionFeedbackPhase)
	{
	case EActionFeedbackPhase::ActionStart:
		OutActionFeedbackData = ActionStartFeedback;
		return true;

	case EActionFeedbackPhase::TrailWindowBegin:
		OutActionFeedbackData = TrailWindowBeginFeedback;
		return true;

	case EActionFeedbackPhase::TrailWindowEnd:
		OutActionFeedbackData = TrailWindowEndFeedback;
		return true;

	case EActionFeedbackPhase::ActionEnd:
		OutActionFeedbackData = ActionEndFeedback;
		return true;

	default:
		break;
	}

	return false;
}

void UCActionFeedbackComponent::PrintActionFeedbackRequestInfo(const FApplyDamageSpecKey& InApplyDamageSpecKey, EActionFeedbackPhase InActionFeedbackPhase) const
{
	FLog::Log(TEXT("==== ActionFeedback Request ====="));
	FLog::Log(TEXT("------ ApplyDamage SpecKey ------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AttachmentType"), *UEnum::GetValueAsString(InApplyDamageSpecKey.AttachmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("EquipmentType"), *UEnum::GetValueAsString(InApplyDamageSpecKey.EquipmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(InApplyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("ActionIndex"), InApplyDamageSpecKey.ActionIndex));
	FLog::Log(TEXT("----- ActionFeedback Phase ------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionFeedbackPhase"), *UEnum::GetValueAsString(InActionFeedbackPhase)));
	FLog::Log(TEXT("================================="));
}

void UCActionFeedbackComponent::PrintActionFeedbackDataInfo(const FActionFeedbackData& InActionFeedbackData) const
{
	FLog::Log(TEXT("====== ActionFeedback Data ======"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionVFX"), *GetNameSafe(InActionFeedbackData.ActionVFX)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionSFX"), *GetNameSafe(InActionFeedbackData.ActionSFX)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bTrailActive"), InActionFeedbackData.bTrailActive ? TEXT("true") : TEXT("false")));
	FLog::Log(TEXT("================================="));
}

void UCActionFeedbackComponent::PrintActionTrailInfo(bool bActive, const ACAttachment* InAttachment) const
{
	FLog::Log(TEXT("======= ActionTrail Info ========"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerActor"), *GetNameSafe(OwnerActor_Cached)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Attachment"), *GetNameSafe(InAttachment)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TrailActive"), bActive ? TEXT("true") : TEXT("false")));
	FLog::Log(TEXT("================================="));
}
