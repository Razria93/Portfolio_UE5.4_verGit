#include "Component/CActionFeedbackComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

#include "Component/CWeaponComponent.h"
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

	FCombatFeedbackProfiling::RecordActionFeedbackRequest();

	if (FCombatFeedbackProfiling::ShouldSkipEnemyCombatFeedback(OwnerCharacter_Injected))
	{
		FCombatFeedbackProfiling::RecordActionFeedbackSkipped();
		return;
	}

	// PrintActionFeedbackRequestInfo(InActionFeedbackRequest);

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
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!IsValid(GetWorld())) return false;
	if (InActionFeedbackRequest.ActionFeedbackTiming == EActionFeedbackTiming::None) return false;

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
		// FLog::Log(TEXT("[ActionFeedback] Trail | No Matched Data")); // Invalid
		return;
	}

	if (bestMatchCount > 1)
	{
		FLog::Log(TEXT("[ActionFeedback] Duplicate highest-priority trail feedback matches")); // Error
		return;
	}

	// FLog::Log(TEXT("[ActionFeedback] Trail | Matched Data")); // Valid
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
		// FLog::Log(TEXT("[ActionFeedback] VFX | No Matched Data"));
		return;
	}

	TSet<FActionVFXExecutionKey> executionKeys;

	for (const FActionVFXFeedbackData* data : matchedDatas)
	{
		const FActionVFXExecutionKey executionKey = BuildActionVFXExecutionKey(*data);

		if (executionKeys.Contains(executionKey))
		{
			FLog::Log(TEXT("[ActionFeedback] Duplicate VFX execution key skipped"));
			continue;
		}

		// FLog::Log(TEXT("[ActionFeedback] VFX | Matched Data"));
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
		// FLog::Log(TEXT("[ActionFeedback] SFX | No Matched Data"));
		return;
	}

	TSet<FActionSFXExecutionKey> executionKeys;

	for (const FActionSFXFeedbackData* data : matchedDatas)
	{
		const FActionSFXExecutionKey executionKey = BuildActionSFXExecutionKey(*data);

		if (executionKeys.Contains(executionKey))
		{
			FLog::Log(TEXT("[ActionFeedback] Duplicate SFX execution key skipped"));
			continue;
		}

		// FLog::Log(TEXT("[ActionFeedback] SFX | Matched Data"));
		executionKeys.Add(executionKey);
		PlayActionSFX(*data);
	}
}

void UCActionFeedbackComponent::PlayActionVFX(const FActionVFXFeedbackData& InActionVFXFeedbackData)
{
	if (!IsValid(InActionVFXFeedbackData.VFX)) return;
	if (!IsValid(OwnerCharacter_Injected)) return;

	switch (InActionVFXFeedbackData.VFXPlayType)
	{
	case EActionVFXPlayType::Once:
	{
		FCombatFeedbackProfiling::RecordActionVFX();

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

		// PrintActionVFXInfo(InActionVFXFeedbackData);

		return;
	}

	case EActionVFXPlayType::Loop:
	{
		// TODO: Implement Loop
		return;
	}

	default:
		return;
	}
}

void UCActionFeedbackComponent::PlayActionSFX(const FActionSFXFeedbackData& InActionSFXFeedbackData)
{
	if (!IsValid(InActionSFXFeedbackData.SFX)) return;
	if (!IsValid(OwnerCharacter_Injected)) return;

	switch (InActionSFXFeedbackData.SFXPlayType)
	{
	case EActionSFXPlayType::Once:
	{
		FCombatFeedbackProfiling::RecordActionSFX();

		UGameplayStatics::PlaySoundAtLocation(
			this,
			InActionSFXFeedbackData.SFX,
			OwnerCharacter_Injected->GetActorLocation());

		// PrintActionSFXInfo(InActionSFXFeedbackData);

		return;
	}

	case EActionSFXPlayType::Loop:
	{
		// TODO: Implement Loop
		return;
	}

	default:
		return;
	}
}

void UCActionFeedbackComponent::ToggleTrailActive(bool bActive)
{
	if (!IsValid(WeaponComp_Injected)) return;

	UObject* uobject = WeaponComp_Injected->GetWeaponActor();
	if (!IsValid(uobject)) return;

	ACWeaponActor* weaponActor = Cast<ACWeaponActor>(uobject);
	if (!IsValid(weaponActor)) return;

	// PrintTrailInfo(bActive, weaponActor);

	FCombatFeedbackProfiling::RecordActionTrail(bActive);

	weaponActor->ToggleTrailActive(bActive);
}

void UCActionFeedbackComponent::PrintActionFeedbackRequestInfo(const FActionFeedbackRequest& InActionFeedbackRequest) const
{
	FLog::Log(TEXT("==== ActionFeedback Request ====="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(InActionFeedbackRequest.ActionFeedbackKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("ActionIndex"), InActionFeedbackRequest.ActionFeedbackKey.ActionIndex));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionFeedbackTiming"), *UEnum::GetValueAsString(InActionFeedbackRequest.ActionFeedbackTiming)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TriggerKey"), *InActionFeedbackRequest.TriggerKey.ToString()));
	FLog::Log(TEXT("================================="));
}

void UCActionFeedbackComponent::PrintActionVFXInfo(const FActionVFXFeedbackData& InActionVFXFeedbackData) const
{
	FLog::Log(TEXT("==== ActionFeedback VFX Info ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("PlayType"), *UEnum::GetValueAsString(InActionVFXFeedbackData.VFXPlayType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Asset"), *GetNameSafe(InActionVFXFeedbackData.VFX)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Socket"), *InActionVFXFeedbackData.SocketName.ToString()));
	FLog::Log(TEXT("================================="));
}

void UCActionFeedbackComponent::PrintActionSFXInfo(const FActionSFXFeedbackData& InActionSFXFeedbackData) const
{
	FLog::Log(TEXT("==== ActionFeedback SFX Info ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("PlayType"), *UEnum::GetValueAsString(InActionSFXFeedbackData.SFXPlayType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Asset"), *GetNameSafe(InActionSFXFeedbackData.SFX)));
	FLog::Log(TEXT("================================="));
}

void UCActionFeedbackComponent::PrintTrailInfo(bool bActive, const ACWeaponActor* InWeaponActor) const
{
	FLog::Log(TEXT("=== ActionFeedback Trail Info ==="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("State"), bActive ? TEXT("Active") : TEXT("Inactive")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponActor"), *GetNameSafe(InWeaponActor)));
	FLog::Log(TEXT("================================="));
}
