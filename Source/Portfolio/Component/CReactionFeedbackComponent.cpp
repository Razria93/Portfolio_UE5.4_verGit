#include "Component/CReactionFeedbackComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"

#include "Core/Profiling/CCombatFeedbackProfiling.h"
#include "Type/CReactionFeedbackStructure.h"

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

	FCombatFeedbackProfiling::RecordReactionFeedbackRequest();

	if (FCombatFeedbackProfiling::ShouldSkipEnemyCombatFeedback(OwnerCharacter_Injected))
	{
		FCombatFeedbackProfiling::RecordReactionFeedbackSkipped();
		return;
	}

	// PrintReactionFeedbackRequestInfo(InReactionFeedbackRequest);
	ExecuteVFXFeedbacks(InReactionFeedbackRequest);
	ExecuteSFXFeedbacks(InReactionFeedbackRequest);
}

void UCReactionFeedbackComponent::ClearRuntimeFeedback()
{
}

bool UCReactionFeedbackComponent::CanPlayReactionFeedback(const FReactionFeedbackRequest& InReactionFeedbackRequest) const
{
	if (InReactionFeedbackRequest.ReactionFeedbackTiming == EReactionFeedbackTiming::None) return false;
	if (InReactionFeedbackRequest.ReactionFeedbackTiming == EReactionFeedbackTiming::Max) return false;

	if (InReactionFeedbackRequest.ReactionFeedbackKey.ReactionType == EReactionType::None) return false;
	if (InReactionFeedbackRequest.ReactionFeedbackKey.ReactionType == EReactionType::All) return false;
	if (InReactionFeedbackRequest.ReactionFeedbackKey.ReactionType == EReactionType::Max) return false;

	return true;
}

bool UCReactionFeedbackComponent::TryCalculateMatchScore(const FReactionFeedbackKey& InDataKey, EReactionFeedbackTiming InDataTiming, FName InDataTriggerKey, const FReactionFeedbackRequest& InReactionFeedbackRequest, int32& OutScore) const
{
	OutScore = 0;

	if (InDataTiming != InReactionFeedbackRequest.ReactionFeedbackTiming)
		return false;

	if (InDataTriggerKey != InReactionFeedbackRequest.TriggerKey)
		return false;

	const FReactionFeedbackKey& requestKey = InReactionFeedbackRequest.ReactionFeedbackKey;

	// ReactionType
	if (InDataKey.ReactionType == requestKey.ReactionType)
	{
		OutScore += ReactionFeedbackScore::ReactionExact;
	}
	else if (InDataKey.ReactionType == EReactionType::All)
	{
		// [Pass] Wildcard match.
	}
	else
	{
		return false;
	}

	// WeaponType
	if (InDataKey.DamageSpecKey.WeaponType == requestKey.DamageSpecKey.WeaponType)
	{
		OutScore += ReactionFeedbackScore::WeaponExact;
	}
	else if (InDataKey.DamageSpecKey.WeaponType == EWeaponType::All)
	{
		// [Pass] Wildcard match.
	}
	else
	{
		return false;
	}

	// ActionType
	if (InDataKey.DamageSpecKey.ActionType == requestKey.DamageSpecKey.ActionType)
	{
		OutScore += ReactionFeedbackScore::ActionExact;
	}
	else if (InDataKey.DamageSpecKey.ActionType == EActionType::All)
	{
		// [Pass] Wildcard match.
	}
	else
	{
		return false;
	}

	// ActionIndex
	if (InDataKey.DamageSpecKey.ActionIndex == requestKey.DamageSpecKey.ActionIndex)
	{
		OutScore += ReactionFeedbackScore::IndexExact;
	}
	else if (InDataKey.DamageSpecKey.ActionIndex == INDEX_NONE)
	{
		// [Pass] Wildcard match.
	}
	else
	{
		return false;
	}

	return true;
}

FReactionVFXExecutionKey UCReactionFeedbackComponent::BuildReactionVFXExecutionKey(const FReactionVFXFeedbackData& InReactionVFXFeedbackData) const
{
	FReactionVFXExecutionKey executionKey;

	executionKey.VFXPlayType = InReactionVFXFeedbackData.VFXPlayType;
	executionKey.VFX = InReactionVFXFeedbackData.VFX;
	executionKey.SocketName = InReactionVFXFeedbackData.SocketName;
	executionKey.RelativeLocation = InReactionVFXFeedbackData.RelativeLocation;
	executionKey.RelativeRotation = InReactionVFXFeedbackData.RelativeRotation;
	executionKey.RelativeScale = InReactionVFXFeedbackData.RelativeScale;

	return executionKey;
}

FReactionSFXExecutionKey UCReactionFeedbackComponent::BuildReactionSFXExecutionKey(const FReactionSFXFeedbackData& InReactionSFXFeedbackData) const
{
	FReactionSFXExecutionKey executionKey;

	executionKey.SFXPlayType = InReactionSFXFeedbackData.SFXPlayType;
	executionKey.SFX = InReactionSFXFeedbackData.SFX;

	return executionKey;
}

void UCReactionFeedbackComponent::ExecuteVFXFeedbacks(const FReactionFeedbackRequest& InReactionFeedbackRequest)
{
	int32 bestScore = INDEX_NONE;
	TArray<const FReactionVFXFeedbackData*> matchedDatas;

	for (const FReactionVFXFeedbackData& data : VFXFeedbackDatas)
	{
		int32 matchScore = INDEX_NONE;

		if (!TryCalculateMatchScore(data.ReactionFeedbackKey, data.ReactionFeedbackTiming, data.TriggerKey, InReactionFeedbackRequest, matchScore))
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
		return;
	}

	TSet<FReactionVFXExecutionKey> executionKeys;

	for (const FReactionVFXFeedbackData* matchedData : matchedDatas)
	{
		if (!matchedData) continue;

		const FReactionVFXExecutionKey executionKey = BuildReactionVFXExecutionKey(*matchedData);

		if (executionKeys.Contains(executionKey))
		{
			FLog::Log(TEXT("[ReactionFeedback] Duplicate VFX execution key skipped"));
			continue;
		}

		// FLog::Log(TEXT("[ReactionFeedback] VFX | Matched Data"));
		executionKeys.Add(executionKey);
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

		if (!TryCalculateMatchScore(data.ReactionFeedbackKey, data.ReactionFeedbackTiming, data.TriggerKey, InReactionFeedbackRequest, matchScore))
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
		return;
	}

	TSet<FReactionSFXExecutionKey> executionKeys;

	for (const FReactionSFXFeedbackData* matchedData : matchedDatas)
	{
		if (!matchedData) continue;

		const FReactionSFXExecutionKey executionKey = BuildReactionSFXExecutionKey(*matchedData);

		if (executionKeys.Contains(executionKey))
		{
			FLog::Log(TEXT("[ReactionFeedback] Duplicate SFX execution key skipped"));
			continue;
		}

		// FLog::Log(TEXT("[ReactionFeedback] SFX | Matched Data"));
		executionKeys.Add(executionKey);
		PlayReactionSFX(*matchedData);
	}
}

void UCReactionFeedbackComponent::PlayReactionVFX(const FReactionVFXFeedbackData& InReactionVFXFeedbackData)
{
	if (!IsValid(InReactionVFXFeedbackData.VFX)) return;
	if (!IsValid(OwnerCharacter_Injected)) return;

	switch (InReactionVFXFeedbackData.VFXPlayType)
	{
	case EReactionVFXPlayType::Once:
	{
		FCombatFeedbackProfiling::RecordReactionVFX();

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

		// PrintReactionVFXInfo(InReactionVFXFeedbackData);

		return;
	}

	case EReactionVFXPlayType::Loop:
	{
		// TODO: Implement Loop
		return;
	}

	default:
		return;
	}
}

void UCReactionFeedbackComponent::PlayReactionSFX(const FReactionSFXFeedbackData& InReactionSFXFeedbackData)
{
	if (!IsValid(InReactionSFXFeedbackData.SFX)) return;
	if (!IsValid(OwnerCharacter_Injected)) return;

	switch (InReactionSFXFeedbackData.SFXPlayType)
	{
	case EReactionSFXPlayType::Once:
	{
		FCombatFeedbackProfiling::RecordReactionSFX();

		UGameplayStatics::PlaySoundAtLocation(
			this,
			InReactionSFXFeedbackData.SFX,
			OwnerCharacter_Injected->GetActorLocation());

		// PrintReactionSFXInfo(InReactionSFXFeedbackData);

		return;
	}

	case EReactionSFXPlayType::Loop:
	{
		// TODO: Implement Loop
		return;
	}

	default:
		return;
	}
}

void UCReactionFeedbackComponent::PrintReactionFeedbackRequestInfo(const FReactionFeedbackRequest& InReactionFeedbackRequest) const
{
	FLog::Log(TEXT("==== ReactionFeedback Request ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(InReactionFeedbackRequest.ReactionFeedbackKey.ReactionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Timing"), *UEnum::GetValueAsString(InReactionFeedbackRequest.ReactionFeedbackTiming)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TriggerKey"), *InReactionFeedbackRequest.TriggerKey.ToString()));
	FLog::Log(TEXT("----------------------------------"));
}

void UCReactionFeedbackComponent::PrintReactionVFXInfo(const FReactionVFXFeedbackData& InReactionVFXFeedbackData) const
{
	FLog::Log(TEXT("==== ReactionFeedback VFX Info ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("PlayType"), *UEnum::GetValueAsString(InReactionVFXFeedbackData.VFXPlayType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Asset"), *GetNameSafe(InReactionVFXFeedbackData.VFX)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Socket"), *InReactionVFXFeedbackData.SocketName.ToString()));
	FLog::Log(TEXT("-----------------------------------"));
}

void UCReactionFeedbackComponent::PrintReactionSFXInfo(const FReactionSFXFeedbackData& InReactionSFXFeedbackData) const
{
	FLog::Log(TEXT("==== ReactionFeedback SFX Info ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("PlayType"), *UEnum::GetValueAsString(InReactionSFXFeedbackData.SFXPlayType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Asset"), *GetNameSafe(InReactionSFXFeedbackData.SFX)));
	FLog::Log(TEXT("-----------------------------------"));
}
