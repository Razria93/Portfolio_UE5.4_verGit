#include "Component/CReactionFeedbackComponent.h"

#include "ProjectGlobal.h"

#include "Core/Debug/FCombatFeedbackDebug.h"
#include "Core/Profiling/CCombatFeedbackProfiling.h"
#include "Type/CReactionFeedbackTypes.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"

// Internal linkage
namespace
{
	namespace ReactionFeedbackScore
	{
		constexpr int32 ReactionExact	= 10000;
		constexpr int32 WeaponExact		= 1000;
		constexpr int32 ActionExact		= 100;
		constexpr int32 IndexExact		= 10;
	}
}

UCReactionFeedbackComponent::UCReactionFeedbackComponent()
{
}

void UCReactionFeedbackComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;

	ValidateRequiredComponentReferences();
}

bool UCReactionFeedbackComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

void UCReactionFeedbackComponent::PlayFeedback(const FReactionFeedbackRequest& InReactionFeedbackRequest)
{
	if (!CanPlayReactionFeedback(InReactionFeedbackRequest)) return;

	FCombatFeedbackDebug::RecordReactionFeedbackRequestAcceptedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest);
	FCombatFeedbackProfiling::RecordReactionFeedbackRequest();

	if (FCombatFeedbackProfiling::ShouldSkipEnemyCombatFeedback(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("RuntimeLODSkipEnemyFeedback"));
		FCombatFeedbackProfiling::RecordReactionFeedbackSkipped();
		return;
	}

	ExecuteVFXFeedbacks(InReactionFeedbackRequest);
	ExecuteSFXFeedbacks(InReactionFeedbackRequest);
}

void UCReactionFeedbackComponent::ClearRuntimeFeedback()
{
}

bool UCReactionFeedbackComponent::CanPlayReactionFeedback(const FReactionFeedbackRequest& InReactionFeedbackRequest) const
{
	if (InReactionFeedbackRequest.ReactionFeedbackTiming == EReactionFeedbackTiming::None)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("InvalidTiming"));
		return false;
	}
	if (InReactionFeedbackRequest.ReactionFeedbackTiming == EReactionFeedbackTiming::Max)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("InvalidTiming"));
		return false;
	}

	if (InReactionFeedbackRequest.ReactionFeedbackMatchKey.ReactionType == EReactionType::None)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("InvalidReactionType"));
		return false;
	}
	if (InReactionFeedbackRequest.ReactionFeedbackMatchKey.ReactionType == EReactionType::All)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("InvalidReactionType"));
		return false;
	}
	if (InReactionFeedbackRequest.ReactionFeedbackMatchKey.ReactionType == EReactionType::Max)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("InvalidReactionType"));
		return false;
	}

	return true;
}

bool UCReactionFeedbackComponent::TryCalculateMatchScore(const FReactionFeedbackMatchKey& InDataKey, EReactionFeedbackTiming InDataTiming, FName InDataTriggerKey, const FReactionFeedbackRequest& InReactionFeedbackRequest, int32& OutScore) const
{
	OutScore = 0;

	if (InDataTiming != InReactionFeedbackRequest.ReactionFeedbackTiming)
		return false;

	if (InDataTriggerKey != InReactionFeedbackRequest.TriggerKey)
		return false;

	const FReactionFeedbackMatchKey& requestKey = InReactionFeedbackRequest.ReactionFeedbackMatchKey;

	if (InDataKey.ReactionType == requestKey.ReactionType)
	{
		OutScore += ReactionFeedbackScore::ReactionExact;
	}
	else if (InDataKey.ReactionType != EReactionType::All)
	{
		return false;
	}

	if (InDataKey.DamageSpecKey.WeaponType == requestKey.DamageSpecKey.WeaponType)
	{
		OutScore += ReactionFeedbackScore::WeaponExact;
	}
	else if (InDataKey.DamageSpecKey.WeaponType != EWeaponType::All)
	{
		return false;
	}

	if (InDataKey.DamageSpecKey.ActionType == requestKey.DamageSpecKey.ActionType)
	{
		OutScore += ReactionFeedbackScore::ActionExact;
	}
	else if (InDataKey.DamageSpecKey.ActionType != EActionType::All)
	{
		return false;
	}

	if (InDataKey.DamageSpecKey.ActionIndex == requestKey.DamageSpecKey.ActionIndex)
	{
		OutScore += ReactionFeedbackScore::IndexExact;
	}
	else if (InDataKey.DamageSpecKey.ActionIndex != INDEX_NONE)
	{
		return false;
	}

	return true;
}

FReactionVFXPlaybackKey UCReactionFeedbackComponent::BuildReactionVFXPlaybackKey(const FReactionVFXFeedbackData& InReactionVFXFeedbackData) const
{
	FReactionVFXPlaybackKey playbackKey;

	playbackKey.VFXPlayType = InReactionVFXFeedbackData.VFXPlayType;
	playbackKey.VFX = InReactionVFXFeedbackData.VFX;
	playbackKey.SocketName = InReactionVFXFeedbackData.SocketName;
	playbackKey.RelativeLocation = InReactionVFXFeedbackData.RelativeLocation;
	playbackKey.RelativeRotation = InReactionVFXFeedbackData.RelativeRotation;
	playbackKey.RelativeScale = InReactionVFXFeedbackData.RelativeScale;

	return playbackKey;
}

FReactionSFXPlaybackKey UCReactionFeedbackComponent::BuildReactionSFXPlaybackKey(const FReactionSFXFeedbackData& InReactionSFXFeedbackData) const
{
	FReactionSFXPlaybackKey playbackKey;

	playbackKey.SFXPlayType = InReactionSFXFeedbackData.SFXPlayType;
	playbackKey.SFX = InReactionSFXFeedbackData.SFX;

	return playbackKey;
}

void UCReactionFeedbackComponent::ExecuteVFXFeedbacks(const FReactionFeedbackRequest& InReactionFeedbackRequest)
{
	int32 bestScore = INDEX_NONE;
	TArray<const FReactionVFXFeedbackData*> matchedDatas;

	for (const FReactionVFXFeedbackData& data : VFXFeedbackDatas)
	{
		int32 matchScore = INDEX_NONE;

		if (!TryCalculateMatchScore(data.ReactionFeedbackMatchKey, data.ReactionFeedbackTiming, data.TriggerKey, InReactionFeedbackRequest, matchScore))
		{
			continue;
		}

		if (matchScore < bestScore) continue;

		// New-High score: Reset and Update List
		if (matchScore > bestScore)
		{
			bestScore = matchScore;
			matchedDatas.Reset();
			matchedDatas.Add(&data);
			continue;
		}

		// Tie score: Add to list
		matchedDatas.Add(&data);
	}

	if (matchedDatas.Num() <= 0)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackChannelRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("VFX"), TEXT("NoMatch"));
		return;
	}

	FCombatFeedbackDebug::RecordReactionFeedbackChannelMatchedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("VFX"), matchedDatas.Num());
	TSet<FReactionVFXPlaybackKey> playbackKeys;

	for (const FReactionVFXFeedbackData* matchedData : matchedDatas)
	{
		if (!matchedData)
		{
			FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), nullptr, TEXT("InvalidMatchedData"));
			continue;
		}

		const FReactionVFXPlaybackKey playbackKey = BuildReactionVFXPlaybackKey(*matchedData);

		if (playbackKeys.Contains(playbackKey))
		{
			FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), matchedData->VFX, TEXT("DuplicatePlaybackKey"));
			continue;
		}

		playbackKeys.Add(playbackKey);
		PlayReactionVFX(*matchedData);
	}
}

void UCReactionFeedbackComponent::ExecuteSFXFeedbacks(const FReactionFeedbackRequest& InReactionFeedbackRequest)
{
	int32 bestScore = INDEX_NONE;
	TArray<const FReactionSFXFeedbackData*> matchedDatas;

	for (const FReactionSFXFeedbackData& data : SFXFeedbackDatas)
	{
		int32 matchScore = INDEX_NONE;

		if (!TryCalculateMatchScore(data.ReactionFeedbackMatchKey, data.ReactionFeedbackTiming, data.TriggerKey, InReactionFeedbackRequest, matchScore))
		{
			continue;
		}

		if (matchScore < bestScore) continue;

		// New-High score: Reset and Update List
		if (matchScore > bestScore)
		{
			bestScore = matchScore;
			matchedDatas.Reset();
			matchedDatas.Add(&data);
			continue;
		}

		// Tie score: Add to list
		matchedDatas.Add(&data);
	}

	if (matchedDatas.Num() <= 0)
	{
		FCombatFeedbackDebug::RecordReactionFeedbackChannelRejectedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("SFX"), TEXT("NoMatch"));
		return;
	}

	FCombatFeedbackDebug::RecordReactionFeedbackChannelMatchedForAudit(OwnerCharacter_Injected, this, InReactionFeedbackRequest, TEXT("SFX"), matchedDatas.Num());
	TSet<FReactionSFXPlaybackKey> playbackKeys;

	for (const FReactionSFXFeedbackData* matchedData : matchedDatas)
	{
		if (!matchedData)
		{
			FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), nullptr, TEXT("InvalidMatchedData"));
			continue;
		}

		const FReactionSFXPlaybackKey playbackKey = BuildReactionSFXPlaybackKey(*matchedData);

		if (playbackKeys.Contains(playbackKey))
		{
			FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), matchedData->SFX, TEXT("DuplicatePlaybackKey"));
			continue;
		}

		playbackKeys.Add(playbackKey);
		PlayReactionSFX(*matchedData);
	}
}

void UCReactionFeedbackComponent::PlayReactionVFX(const FReactionVFXFeedbackData& InReactionVFXFeedbackData)
{
	if (!IsValid(InReactionVFXFeedbackData.VFX))
	{
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InReactionVFXFeedbackData.VFX, TEXT("InvalidAsset"));
		return;
	}
	if (!IsValid(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InReactionVFXFeedbackData.VFX, TEXT("InvalidOwner"));
		return;
	}

	switch (InReactionVFXFeedbackData.VFXPlayType)
	{
	case EReactionVFXPlayType::Once:
	{
		FCombatFeedbackProfiling::RecordReactionVFX();
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationPlayedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InReactionVFXFeedbackData.VFX, TEXT("SpawnOnce"));

		UNiagaraFunctionLibrary::SpawnSystemAttached(
			InReactionVFXFeedbackData.VFX,
			OwnerCharacter_Injected->GetMesh(),
			InReactionVFXFeedbackData.SocketName,
			InReactionVFXFeedbackData.RelativeLocation,
			InReactionVFXFeedbackData.RelativeRotation,
			InReactionVFXFeedbackData.RelativeScale,
			EAttachLocation::KeepRelativeOffset,
			true,
			ENCPoolMethod::None);

		return;
	}

	case EReactionVFXPlayType::Loop:
	{
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InReactionVFXFeedbackData.VFX, TEXT("LoopNotImplemented"));
		return;
	}

	default:
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), InReactionVFXFeedbackData.VFX, TEXT("UnsupportedPlayType"));
		return;
	}
}
void UCReactionFeedbackComponent::PlayReactionSFX(const FReactionSFXFeedbackData& InReactionSFXFeedbackData)
{
	if (!IsValid(InReactionSFXFeedbackData.SFX))
	{
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InReactionSFXFeedbackData.SFX, TEXT("InvalidAsset"));
		return;
	}
	if (!IsValid(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InReactionSFXFeedbackData.SFX, TEXT("InvalidOwner"));
		return;
	}

	switch (InReactionSFXFeedbackData.SFXPlayType)
	{
	case EReactionSFXPlayType::Once:
	{
		FCombatFeedbackProfiling::RecordReactionSFX();
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationPlayedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InReactionSFXFeedbackData.SFX, TEXT("PlayOnce"));

		UGameplayStatics::PlaySoundAtLocation(
			this,
			InReactionSFXFeedbackData.SFX,
			OwnerCharacter_Injected->GetActorLocation());

		return;
	}

	case EReactionSFXPlayType::Loop:
	{
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InReactionSFXFeedbackData.SFX, TEXT("LoopNotImplemented"));
		return;
	}

	default:
		FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), InReactionSFXFeedbackData.SFX, TEXT("UnsupportedPlayType"));
		return;
	}
}
