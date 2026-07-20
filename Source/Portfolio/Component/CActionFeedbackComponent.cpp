#include "Component/CActionFeedbackComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

#include "Component/CWeaponComponent.h"
#include "Core/Debug/FCombatFeedbackDebug.h"
#include "Core/Profiling/CCombatFeedbackProfiling.h"
#include "Weapon/CWeaponActor.h"

UCActionFeedbackComponent::UCActionFeedbackComponent()
{
}

void UCActionFeedbackComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	WeaponComp_Injected = InReferences.WeaponComponent;

	ValidateRequiredComponentReferences();
}

bool UCActionFeedbackComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ WeaponComp_Injected, TEXT("UCWeaponComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

void UCActionFeedbackComponent::PlayFeedback(const FActionFeedbackRequest& InActionFeedbackRequest)
{
	if (!CanPlayActionFeedback(InActionFeedbackRequest)) return;

	FCombatFeedbackDebug::RecordActionFeedbackRequestAcceptedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest);
	FCombatFeedbackProfiling::RecordActionFeedbackRequest();

	if (FCombatFeedbackProfiling::ShouldSkipEnemyCombatFeedback(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordActionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("RuntimeLODSkipEnemyFeedback"));
		FCombatFeedbackProfiling::RecordActionFeedbackSkipped();
		return;
	}

	ExecuteTrailFeedbacks(InActionFeedbackRequest);
	ExecuteVFXFeedbacks(InActionFeedbackRequest);
	ExecuteSFXFeedbacks(InActionFeedbackRequest);
}

void UCActionFeedbackComponent::ClearRuntimeFeedback()
{
	ToggleTrailActive(false);
}

bool UCActionFeedbackComponent::CanPlayActionFeedback(const FActionFeedbackRequest& InActionFeedbackRequest) const
{
	if (!IsValid(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordActionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("InvalidOwner"));
		return false;
	}
	if (!IsValid(GetWorld()))
	{
		FCombatFeedbackDebug::RecordActionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("InvalidWorld"));
		return false;
	}
	if (InActionFeedbackRequest.ActionFeedbackTiming == EActionFeedbackTiming::None)
	{
		FCombatFeedbackDebug::RecordActionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("InvalidTiming"));
		return false;
	}

	return true;
}

EActionFeedbackMatchTier UCActionFeedbackComponent::CalculateMatchTier(const FActionFeedbackKey& InDataKey, EActionFeedbackTiming InDataTiming, FName InDataTriggerKey, const FActionFeedbackRequest& InActionFeedbackRequest) const
{
	// Exact
	if (InDataTiming != InActionFeedbackRequest.ActionFeedbackTiming)
		return EActionFeedbackMatchTier::None;

	if (InDataTriggerKey != InActionFeedbackRequest.TriggerKey)
		return EActionFeedbackMatchTier::None;

	// Exact & Wildcard
	const bool bActionExact = (InDataKey.ActionType == InActionFeedbackRequest.ActionFeedbackKey.ActionType);
	const bool bActionAny = (InDataKey.ActionType == EActionType::All);

	const bool bIndexExact = (InDataKey.ActionIndex == InActionFeedbackRequest.ActionFeedbackKey.ActionIndex);
	const bool bIndexAny = (InDataKey.ActionIndex == INDEX_NONE);

	// Tier 0
	if (bActionExact && bIndexExact)
		return EActionFeedbackMatchTier::ExactActionExactIndex;

	// Tier 1	
	if (bActionExact && bIndexAny)
		return EActionFeedbackMatchTier::ExactActionAnyIndex;

	// Tier 2
	if (bActionAny && bIndexAny)
		return EActionFeedbackMatchTier::AnyActionAnyIndex;

	return EActionFeedbackMatchTier::None;
}

FActionVFXExecutionKey UCActionFeedbackComponent::BuildActionVFXExecutionKey(const FActionVFXFeedbackData& InActionVFXFeedbackData) const
{
	FActionVFXExecutionKey executionKey;

	executionKey.ActionFeedbackKey = InActionVFXFeedbackData.ActionFeedbackKey;
	executionKey.ActionFeedbackTiming = InActionVFXFeedbackData.ActionFeedbackTiming;
	executionKey.TriggerKey = InActionVFXFeedbackData.TriggerKey;
	executionKey.VFXPlayType = InActionVFXFeedbackData.VFXPlayType;
	executionKey.VFX = InActionVFXFeedbackData.VFX;
	executionKey.SocketName = InActionVFXFeedbackData.SocketName;
	executionKey.RelativeLocation = InActionVFXFeedbackData.RelativeLocation;
	executionKey.RelativeRotation = InActionVFXFeedbackData.RelativeRotation;
	executionKey.RelativeScale = InActionVFXFeedbackData.RelativeScale;

	return executionKey;
}

FActionSFXExecutionKey UCActionFeedbackComponent::BuildActionSFXExecutionKey(const FActionSFXFeedbackData& InActionSFXFeedbackData) const
{
	FActionSFXExecutionKey executionKey;

	executionKey.ActionFeedbackKey = InActionSFXFeedbackData.ActionFeedbackKey;
	executionKey.ActionFeedbackTiming = InActionSFXFeedbackData.ActionFeedbackTiming;
	executionKey.TriggerKey = InActionSFXFeedbackData.TriggerKey;
	executionKey.SFXPlayType = InActionSFXFeedbackData.SFXPlayType;
	executionKey.SFX = InActionSFXFeedbackData.SFX;

	return executionKey;
}

void UCActionFeedbackComponent::ExecuteTrailFeedbacks(const FActionFeedbackRequest& InActionFeedbackRequest)
{
	// Cached Score and Data
	EActionFeedbackMatchTier bestTier = EActionFeedbackMatchTier::None;
	const FTrailFeedbackData* bestData = nullptr;
	int32 bestMatchCount = 0;

	for (const FTrailFeedbackData& trailFeedbackData : TrailFeedbackDatas)
	{
		const EActionFeedbackMatchTier matchTier = CalculateMatchTier(trailFeedbackData.ActionFeedbackKey, trailFeedbackData.ActionFeedbackTiming, trailFeedbackData.TriggerKey, InActionFeedbackRequest);

		if (matchTier == EActionFeedbackMatchTier::None) continue;
		if (static_cast<uint8>(matchTier) < static_cast<uint8>(bestTier)) continue;

		// New high score: Reset and update
		if (static_cast<uint8>(matchTier) > static_cast<uint8>(bestTier))
		{
			bestTier = matchTier;
			bestData = &trailFeedbackData;
			bestMatchCount = 1;
			continue;
		}

		// Tie: Error (Architect Miss)
		++bestMatchCount;
	}

	if (!bestData)
	{
		FCombatFeedbackDebug::RecordActionFeedbackChannelRejectedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("Trail"), TEXT("NoMatch"));
		return;
	}

	if (bestMatchCount > 1)
	{
		FCombatFeedbackDebug::RecordActionFeedbackChannelRejectedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("Trail"), TEXT("DuplicateBestMatch"), bestMatchCount);
		return;
	}

	FCombatFeedbackDebug::RecordActionFeedbackChannelMatchedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("Trail"), bestMatchCount);
	ToggleTrailActive(bestData->bTrailActive);
}

void UCActionFeedbackComponent::ExecuteVFXFeedbacks(const FActionFeedbackRequest& InActionFeedbackRequest)
{
	EActionFeedbackMatchTier bestTier = EActionFeedbackMatchTier::None;
	TArray<const FActionVFXFeedbackData*> matchedDatas;

	for (const FActionVFXFeedbackData& actionVFXFeedbackData : VFXFeedbackDatas)
	{
		const EActionFeedbackMatchTier matchTier = CalculateMatchTier(actionVFXFeedbackData.ActionFeedbackKey, actionVFXFeedbackData.ActionFeedbackTiming, actionVFXFeedbackData.TriggerKey, InActionFeedbackRequest);

		if (matchTier == EActionFeedbackMatchTier::None) continue;
		if (static_cast<uint8>(matchTier) < static_cast<uint8>(bestTier)) continue;

		// New high score: Reset and update
		if (static_cast<uint8>(matchTier) > static_cast<uint8>(bestTier))
		{
			bestTier = matchTier;
			matchedDatas.Reset();
			matchedDatas.Add(&actionVFXFeedbackData);
			continue;
		}

		// Tie: Add to list
		matchedDatas.Add(&actionVFXFeedbackData);
	}

	if (matchedDatas.Num() <= 0)
	{
		FCombatFeedbackDebug::RecordActionFeedbackChannelRejectedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("VFX"), TEXT("NoMatch"));
		return;
	}

	FCombatFeedbackDebug::RecordActionFeedbackChannelMatchedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("VFX"), matchedDatas.Num());
	TSet<FActionVFXExecutionKey> executionKeys;

	for (const FActionVFXFeedbackData* data : matchedDatas)
	{
		const FActionVFXExecutionKey executionKey = BuildActionVFXExecutionKey(*data);

		if (executionKeys.Contains(executionKey))
		{
			FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), data ? data->VFX : nullptr, TEXT("DuplicateExecutionKey"));
			continue;
		}

		executionKeys.Add(executionKey);
		PlayActionVFX(*data);
	}
}

void UCActionFeedbackComponent::ExecuteSFXFeedbacks(const FActionFeedbackRequest& InActionFeedbackRequest)
{
	EActionFeedbackMatchTier bestTier = EActionFeedbackMatchTier::None;
	TArray<const FActionSFXFeedbackData*> matchedDatas;

	for (const FActionSFXFeedbackData& actionSFXFeedbackData : SFXFeedbackDatas)
	{
		const EActionFeedbackMatchTier matchTier = CalculateMatchTier(actionSFXFeedbackData.ActionFeedbackKey, actionSFXFeedbackData.ActionFeedbackTiming, actionSFXFeedbackData.TriggerKey, InActionFeedbackRequest);

		if (matchTier == EActionFeedbackMatchTier::None) continue;
		if (static_cast<uint8>(matchTier) < static_cast<uint8>(bestTier)) continue;

		// New high score: Reset and update
		if (static_cast<uint8>(matchTier) > static_cast<uint8>(bestTier))
		{
			bestTier = matchTier;
			matchedDatas.Reset();
			matchedDatas.Add(&actionSFXFeedbackData);
			continue;
		}

		// Tie: Add to list
		matchedDatas.Add(&actionSFXFeedbackData);
	}

	if (matchedDatas.Num() <= 0)
	{
		FCombatFeedbackDebug::RecordActionFeedbackChannelRejectedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("SFX"), TEXT("NoMatch"));
		return;
	}

	FCombatFeedbackDebug::RecordActionFeedbackChannelMatchedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("SFX"), matchedDatas.Num());
	TSet<FActionSFXExecutionKey> executionKeys;

	for (const FActionSFXFeedbackData* data : matchedDatas)
	{
		const FActionSFXExecutionKey executionKey = BuildActionSFXExecutionKey(*data);

		if (executionKeys.Contains(executionKey))
		{
			FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), data ? data->SFX : nullptr, TEXT("DuplicateExecutionKey"));
			continue;
		}

		executionKeys.Add(executionKey);
		PlayActionSFX(*data);
	}
}

void UCActionFeedbackComponent::PlayActionVFX(const FActionVFXFeedbackData& InActionVFXFeedbackData)
{
	if (!IsValid(InActionVFXFeedbackData.VFX))
	{
		FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InActionVFXFeedbackData.VFX, TEXT("InvalidAsset"));
		return;
	}
	if (!IsValid(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InActionVFXFeedbackData.VFX, TEXT("InvalidOwner"));
		return;
	}

	switch (InActionVFXFeedbackData.VFXPlayType)
	{
	case EActionVFXPlayType::Once:
	{
		FCombatFeedbackProfiling::RecordActionVFX();
		FCombatFeedbackDebug::RecordActionFeedbackPresentationPlayedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InActionVFXFeedbackData.VFX, TEXT("SpawnOnce"));

		UNiagaraFunctionLibrary::SpawnSystemAttached(
			InActionVFXFeedbackData.VFX,
			OwnerCharacter_Injected->GetMesh(),
			InActionVFXFeedbackData.SocketName,
			InActionVFXFeedbackData.RelativeLocation,
			InActionVFXFeedbackData.RelativeRotation,
			InActionVFXFeedbackData.RelativeScale,
			EAttachLocation::KeepRelativeOffset,
			true,
			ENCPoolMethod::None);

		return;
	}

	case EActionVFXPlayType::Loop:
	{
		// TODO: Implement Loop
		FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InActionVFXFeedbackData.VFX, TEXT("LoopNotImplemented"));
		return;
	}

	default:
		FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InActionVFXFeedbackData.VFX, TEXT("UnsupportedPlayType"));
		return;
	}
}

void UCActionFeedbackComponent::PlayActionSFX(const FActionSFXFeedbackData& InActionSFXFeedbackData)
{
	if (!IsValid(InActionSFXFeedbackData.SFX))
	{
		FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InActionSFXFeedbackData.SFX, TEXT("InvalidAsset"));
		return;
	}
	if (!IsValid(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InActionSFXFeedbackData.SFX, TEXT("InvalidOwner"));
		return;
	}

	switch (InActionSFXFeedbackData.SFXPlayType)
	{
	case EActionSFXPlayType::Once:
	{
		FCombatFeedbackProfiling::RecordActionSFX();
		FCombatFeedbackDebug::RecordActionFeedbackPresentationPlayedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InActionSFXFeedbackData.SFX, TEXT("PlayOnce"));

		UGameplayStatics::PlaySoundAtLocation(
			this,
			InActionSFXFeedbackData.SFX,
			OwnerCharacter_Injected->GetActorLocation());

		return;
	}

	case EActionSFXPlayType::Loop:
	{
		// TODO: Implement Loop
		FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InActionSFXFeedbackData.SFX, TEXT("LoopNotImplemented"));
		return;
	}

	default:
		FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InActionSFXFeedbackData.SFX, TEXT("UnsupportedPlayType"));
		return;
	}
}

void UCActionFeedbackComponent::ToggleTrailActive(bool bActive)
{
	if (!IsValid(WeaponComp_Injected))
	{
		FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("Trail"), nullptr, TEXT("InvalidWeaponComponent"));
		return;
	}

	UObject* uobject = WeaponComp_Injected->GetWeaponActor();
	if (!IsValid(uobject))
	{
		FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("Trail"), uobject, TEXT("InvalidWeaponActorObject"));
		return;
	}

	ACWeaponActor* weaponActor = Cast<ACWeaponActor>(uobject);
	if (!IsValid(weaponActor))
	{
		FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("Trail"), uobject, TEXT("InvalidWeaponActor"));
		return;
	}

	FCombatFeedbackProfiling::RecordActionTrail(bActive);
	FCombatFeedbackDebug::RecordActionFeedbackPresentationPlayedForAudit(OwnerCharacter_Injected, this, TEXT("Trail"), weaponActor, bActive ? TEXT("Activate") : TEXT("Deactivate"));

	weaponActor->ToggleTrailActive(bActive);
}
