#include "Component/CActionFeedbackComponent.h"

#include "ProjectGlobal.h"

#include "Component/CWeaponComponent.h"
#include "Core/Debug/FCombatFeedbackDebug.h"
#include "Core/Profiling/CCombatFeedbackProfiling.h"
#include "Weapon/CWeaponActor.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

UCActionFeedbackComponent::UCActionFeedbackComponent()
{
}

// Component Reference

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

// Entry

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

// Query

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

// Matching

EActionFeedbackMatchTier UCActionFeedbackComponent::CalculateMatchTier(const FActionFeedbackMatchKey& InDataKey, EActionFeedbackTiming InDataTiming, FName InDataTriggerKey, const FActionFeedbackRequest& InActionFeedbackRequest) const
{
	if (InDataTiming != InActionFeedbackRequest.ActionFeedbackTiming)
		return EActionFeedbackMatchTier::None;

	if (InDataTriggerKey != InActionFeedbackRequest.TriggerKey)
		return EActionFeedbackMatchTier::None;

	const bool bActionExact = (InDataKey.ActionType == InActionFeedbackRequest.ActionFeedbackMatchKey.ActionType);
	const bool bActionAny = (InDataKey.ActionType == EActionType::All);

	const bool bIndexExact = (InDataKey.ActionIndex == InActionFeedbackRequest.ActionFeedbackMatchKey.ActionIndex);
	const bool bIndexAny = (InDataKey.ActionIndex == INDEX_NONE);

	if (bActionExact && bIndexExact)
		return EActionFeedbackMatchTier::ExactActionExactIndex;

	if (bActionExact && bIndexAny)
		return EActionFeedbackMatchTier::ExactActionAnyIndex;

	if (bActionAny && bIndexAny)
		return EActionFeedbackMatchTier::AnyActionAnyIndex;

	return EActionFeedbackMatchTier::None;
}

// Runtime Key / Playback Key

FActionVFXPlaybackKey UCActionFeedbackComponent::BuildActionVFXPlaybackKey(const FActionVFXFeedbackData& InActionVFXFeedbackData) const
{
	FActionVFXPlaybackKey playbackKey;

	playbackKey.VFXPlayType = InActionVFXFeedbackData.VFXPlayType;
	playbackKey.VFX = InActionVFXFeedbackData.VFX;
	playbackKey.SocketName = InActionVFXFeedbackData.SocketName;
	playbackKey.RelativeLocation = InActionVFXFeedbackData.RelativeLocation;
	playbackKey.RelativeRotation = InActionVFXFeedbackData.RelativeRotation;
	playbackKey.RelativeScale = InActionVFXFeedbackData.RelativeScale;

	return playbackKey;
}

FActionSFXPlaybackKey UCActionFeedbackComponent::BuildActionSFXPlaybackKey(const FActionSFXFeedbackData& InActionSFXFeedbackData) const
{
	FActionSFXPlaybackKey playbackKey;

	playbackKey.SFXPlayType = InActionSFXFeedbackData.SFXPlayType;
	playbackKey.SFX = InActionSFXFeedbackData.SFX;

	return playbackKey;
}

// Execution

void UCActionFeedbackComponent::ExecuteTrailFeedbacks(const FActionFeedbackRequest& InActionFeedbackRequest)
{
	EActionFeedbackMatchTier bestTier = EActionFeedbackMatchTier::None;
	const FActionTrailFeedbackData* bestData = nullptr;
	int32 bestMatchCount = 0;

	for (const FActionTrailFeedbackData& trailFeedbackData : TrailFeedbackDatas)
	{
		const EActionFeedbackMatchTier matchTier = CalculateMatchTier(trailFeedbackData.ActionFeedbackMatchKey, trailFeedbackData.ActionFeedbackTiming, trailFeedbackData.TriggerKey, InActionFeedbackRequest);

		if (matchTier == EActionFeedbackMatchTier::None) continue;
		if (static_cast<uint8>(matchTier) < static_cast<uint8>(bestTier)) continue;

		if (static_cast<uint8>(matchTier) > static_cast<uint8>(bestTier))
		{
			bestTier = matchTier;
			bestData = &trailFeedbackData;
			bestMatchCount = 1;
			continue;
		}

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
		const EActionFeedbackMatchTier matchTier = CalculateMatchTier(actionVFXFeedbackData.ActionFeedbackMatchKey, actionVFXFeedbackData.ActionFeedbackTiming, actionVFXFeedbackData.TriggerKey, InActionFeedbackRequest);

		if (matchTier == EActionFeedbackMatchTier::None) continue;
		if (static_cast<uint8>(matchTier) < static_cast<uint8>(bestTier)) continue;

		if (static_cast<uint8>(matchTier) > static_cast<uint8>(bestTier))
		{
			bestTier = matchTier;
			matchedDatas.Reset();
			matchedDatas.Add(&actionVFXFeedbackData);
			continue;
		}

		matchedDatas.Add(&actionVFXFeedbackData);
	}

	if (matchedDatas.Num() <= 0)
	{
		FCombatFeedbackDebug::RecordActionFeedbackChannelRejectedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("VFX"), TEXT("NoMatch"));
		return;
	}

	FCombatFeedbackDebug::RecordActionFeedbackChannelMatchedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("VFX"), matchedDatas.Num());
	TSet<FActionVFXPlaybackKey> playbackKeys;

	for (const FActionVFXFeedbackData* data : matchedDatas)
	{
		const FActionVFXPlaybackKey playbackKey = BuildActionVFXPlaybackKey(*data);

		if (playbackKeys.Contains(playbackKey))
		{
			FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), data ? data->VFX : nullptr, TEXT("DuplicatePlaybackKey"));
			continue;
		}

		playbackKeys.Add(playbackKey);
		PlayActionVFX(*data);
	}
}

void UCActionFeedbackComponent::ExecuteSFXFeedbacks(const FActionFeedbackRequest& InActionFeedbackRequest)
{
	EActionFeedbackMatchTier bestTier = EActionFeedbackMatchTier::None;
	TArray<const FActionSFXFeedbackData*> matchedDatas;

	for (const FActionSFXFeedbackData& actionSFXFeedbackData : SFXFeedbackDatas)
	{
		const EActionFeedbackMatchTier matchTier = CalculateMatchTier(actionSFXFeedbackData.ActionFeedbackMatchKey, actionSFXFeedbackData.ActionFeedbackTiming, actionSFXFeedbackData.TriggerKey, InActionFeedbackRequest);

		if (matchTier == EActionFeedbackMatchTier::None) continue;
		if (static_cast<uint8>(matchTier) < static_cast<uint8>(bestTier)) continue;

		if (static_cast<uint8>(matchTier) > static_cast<uint8>(bestTier))
		{
			bestTier = matchTier;
			matchedDatas.Reset();
			matchedDatas.Add(&actionSFXFeedbackData);
			continue;
		}

		matchedDatas.Add(&actionSFXFeedbackData);
	}

	if (matchedDatas.Num() <= 0)
	{
		FCombatFeedbackDebug::RecordActionFeedbackChannelRejectedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("SFX"), TEXT("NoMatch"));
		return;
	}

	FCombatFeedbackDebug::RecordActionFeedbackChannelMatchedForAudit(OwnerCharacter_Injected, this, InActionFeedbackRequest, TEXT("SFX"), matchedDatas.Num());
	TSet<FActionSFXPlaybackKey> playbackKeys;

	for (const FActionSFXFeedbackData* data : matchedDatas)
	{
		const FActionSFXPlaybackKey playbackKey = BuildActionSFXPlaybackKey(*data);

		if (playbackKeys.Contains(playbackKey))
		{
			FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), data ? data->SFX : nullptr, TEXT("DuplicatePlaybackKey"));
			continue;
		}

		playbackKeys.Add(playbackKey);
		PlayActionSFX(*data);
	}
}

// Playback

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
