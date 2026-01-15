#include "Component/CReactionComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Reaction/CReaction.h"

#include "Type/CWeaponStructure.h"

UCReactionComponent::UCReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	// Rebuild All
	BuildReactionDataMap(true);
	BuildReactionMap(true);

	PrintReactionDataMapInfo();
}

void UCReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCReactionComponent::AnimNotify_ReactionBegin()
{
	if (IsValid(ActiveReactionExcutor_Cached))
	{
		ActiveReactionExcutor_Cached->OnAnimNotify_ReactionBegin();
	}
}

void UCReactionComponent::AnimNotify_ReactionEnd()
{
	if (IsValid(ActiveReactionExcutor_Cached))
	{
		ActiveReactionExcutor_Cached->OnAnimNotify_ReactionEnd();
	}

	// Clear State
	bIsReaction = false;
	ActiveReactionExcutor_Cached = nullptr;
	ActiveReactionExcutorKey_Cached = nullptr;
	ActiveReactionData_Cached = FReactionData();
	ActiveReactionDataKey_Cached = FReactionDataKey();
}

void UCReactionComponent::RequestReaction(const FTakeDamageResult& InTakeDamageResult)
{
	ProcessReaction(InTakeDamageResult);
}

void UCReactionComponent::ProcessReaction(const FTakeDamageResult& InTakeDamageResult)
{
	// NOTE: Use only the result of the commit

	if (!ValidateRequest(InTakeDamageResult)) return;

	const EReactionType newReactionType = ResolveReactionType(InTakeDamageResult);
	if (newReactionType == EReactionType::None) return;

	FReactionData reactionData;
	if (!ResolveReactionData(InTakeDamageResult.ApplyDamageSpecKey, newReactionType, reactionData)) return;

	UCReaction* newReaction = ResolveReaction(reactionData);
	if (!IsValid(newReaction)) return;

	// Case 01: InValid ActiveReaction
	if (!IsValid(ActiveReactionExcutor_Cached))
	{
		PlayReaction(newReaction, reactionData);
		return;
	}

	// Case 02: Valid ActiveReaction && New reactions disabled
	if (!QueryAcceptNewReaction(ActiveReactionExcutor_Cached, newReaction, ActiveReactionData_Cached, reactionData)) return;

	// Case 03: Valid ActiveReaction && New reactions abled
	ActiveReactionExcutor_Cached->Stop(EReactionStopReason::Interrupted, newReaction);
	PlayReaction(newReaction, reactionData);
}

bool UCReactionComponent::ValidateRequest(const FTakeDamageResult& takeDamageResult) const
{
	if (!IsValid(OwnerCharacter_Cached)) return false;
	if (!takeDamageResult.bAccepted) return false;

	return true;
}

EReactionType UCReactionComponent::ResolveReactionType(const FTakeDamageResult& takeDamageResult)
{
	// Enforce Dead Type if bKilled
	if (takeDamageResult.bKilled || takeDamageResult.bTriggerDeathReaction)
		return EReactionType::Dead;

	if (takeDamageResult.bTriggerHitReaction)
		return EReactionType::Hit;

	return EReactionType::None;
}

bool UCReactionComponent::ResolveReactionData(const FApplyDamageSpecKey& InApplyDamageSpecKey, EReactionType InReactionType, FReactionData& OutReactionData)
{
	FLog::Log(TEXT("====== ResolveReaction Info ========"));

	// Debug Input
	PrintApplyDamageSpecKeyInfo(InApplyDamageSpecKey);

	OutReactionData = FReactionData();

	TArray<FApplyDamageSpecKey> candidateKeys;
	BuildCandidateSpecKeys(InApplyDamageSpecKey, candidateKeys);

	for (const FApplyDamageSpecKey& candidateKey : candidateKeys)
	{
		FReactionDataKey reactionDataKey;
		reactionDataKey.ApplyDamageSpecKey = candidateKey;
		reactionDataKey.ReactionType = InReactionType;

		// [Debug] ReactionDataKey
		PrintReactionDataKeyInfo(reactionDataKey);

		// 1) Find ReactionData
		const FReactionData* reactionDataPtr = ReactionDataMap.Find(reactionDataKey);
		if (!reactionDataPtr)
		{
			// InValid ReactionData
			FLog::Log(TEXT("Not Found"));
			continue;
		}

		// Valid ReactionData
		const FReactionData& reactionData = *reactionDataPtr;
		OutReactionData = reactionData;

		// [Debug] ReactionData
		PrintReactionDataInfo(reactionData);

		return true;
	}

	// [Debug] InValid ReactionData in ReactionDataMap
	FLog::Log(TEXT("[ResolveReactionData] Fail to resolve ReactionData"));
	FLog::Log(TEXT("===================================="));

	return false;
}

UCReaction* UCReactionComponent::ResolveReaction(const FReactionData& InReactionData)
{
	// 1) Try reuse cached Reaction; return if valid
	UCReaction* foundReaction = FindReaction(InReactionData.ReactionExecutorKey.Get());
	if (IsValid(foundReaction)) return foundReaction;

	// 2) [Policy] Try create and cache Reaction; return if valid
	UCReaction* newReaction = CreateReaction(InReactionData.ReactionExecutorKey);
	if (IsValid(newReaction)) return newReaction;

	// [Debug] ReactionData is Valid; but Find and Create Failed
	FLog::Log(TEXT("[ResolveReaction] Valid ReactionData, but failed to resolve Reaction (find/create failed)."));

	return nullptr;
}

bool UCReactionComponent::QueryAcceptNewReaction(UCReaction* InActiveReaction, UCReaction* InNewReaction, const FReactionData& InActiveReactionData, const FReactionData& InNewReactionData)
{
	if (!InNewReactionData.IsValidMinimal()) return false;
	if (!InActiveReactionData.IsValidMinimal()) return true; // FirstReaction

	// [POLICY] Duplicate ReactionDataKey: Currently set to 'ignore'.
	// (Options: ignore | restart | stop-then-play)
	if (InNewReactionData.ReactionDataKey == InActiveReactionData.ReactionDataKey) return false;

	if (!InActiveReaction->CanBeInterrupted(InActiveReactionData, InNewReactionData)) return false;
	if (!InNewReaction->CanInterrupt(InActiveReactionData, InNewReactionData)) return false;

	return true;
}

void UCReactionComponent::PlayReaction(UCReaction* InNewReaction, const FReactionData& InReactionData)
{
	if (InNewReaction->PlayReaction(InReactionData))
		ChangeActiveReaction(InNewReaction, InReactionData);
}

void UCReactionComponent::ChangeActiveReaction(UCReaction* InNewReaction, const FReactionData& InReactionData)
{
	if (!IsValid(InNewReaction)) return;


	bIsReaction = true;

	ActiveReactionDataKey_Cached = InReactionData.ReactionDataKey;
	ActiveReactionData_Cached = InReactionData;

	ActiveReactionExcutorKey_Cached = InReactionData.ReactionExecutorKey.Get();
	ActiveReactionExcutor_Cached = InNewReaction;
}

void UCReactionComponent::BuildReactionDataMap(bool bRebuildAll)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	// bRebuildAll == true: Rebuild 
	// bRebuildAll == false: Append

	if (bRebuildAll)
	{
		ReactionDataMap.Reset();
	}

	for (const FReactionData& reactionData : ReactionDatas)
	{
		if (!reactionData.IsValidMinimal())
			continue;

		FReactionDataKey reactionDataKey = reactionData.ReactionDataKey;

		if (ReactionDataMap.Contains(reactionDataKey))
		{
			if (bRebuildAll)
			{
				FLog::Log(TEXT("[Duplicate key] Overwrite Value"));
				ReactionDataMap[reactionDataKey] = reactionData;
			}
			else // bRebuildAll == false
			{
				// [POLICY] Duplicate ReactionDataKey: Currently set to 'skip'.
				// (Options: ignore | restart | stop-then-play)
				continue;
			}
		}
		else // Contains(reactionDataKey) == false
		{
			ReactionDataMap.Add(reactionDataKey, reactionData);
		}
	}
}

void UCReactionComponent::BuildReactionMap(bool bRebuildAll)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	// bRebuildAll == true: Rebuild 
	// bRebuildAll == false: Append

	if (bRebuildAll)
	{
		ReactionExcutorMap.Reset();
	}

	for (const TSubclassOf<class UCReaction> reactionClass : ReactionClasses)
	{
		if (!IsValid(reactionClass)) continue;

		UClass* reactionExcutorKey = reactionClass.Get();

		// 1) Find existing cached Reaction
		if (!bRebuildAll)
		{
			const UCReaction* foundReaction = FindReaction(reactionExcutorKey);
			if (IsValid(foundReaction)) continue;
		}

		// 2) Create and Cache Reaction
		UCReaction* createdReaction = CreateReaction(reactionClass);
		if (!IsValid(createdReaction))
		{
			// [Debug] ReactionData is Valid; but Find and Create Failed
			FLog::Log(TEXT("[Reaction] Valid ReactionData, but failed to find/create Reaction."));
			continue;
		}
	}
}

void UCReactionComponent::BuildCandidateSpecKeys(const FApplyDamageSpecKey& InApplyDamageSpecKey, TArray<FApplyDamageSpecKey>& OutApplyDamageSpecKeys) const
{
	OutApplyDamageSpecKeys.Reset();

	// 1) 'Exact' Key
	OutApplyDamageSpecKeys.Add(InApplyDamageSpecKey);

	// 2) 'Index Any' Key
	{
		FApplyDamageSpecKey candidateKey = InApplyDamageSpecKey;
		candidateKey.ActionIndex = INDEX_NONE;
		OutApplyDamageSpecKeys.Add(candidateKey);
	}

	// 3) 'Action + Index Any' Key
	{
		FApplyDamageSpecKey candidateKey = InApplyDamageSpecKey;
		candidateKey.ActionType = EActionType::All;
		candidateKey.ActionIndex = INDEX_NONE;
		OutApplyDamageSpecKeys.Add(candidateKey);
	}

	// 4) 'Equipment + Action + Index Any' Key
	{
		FApplyDamageSpecKey candidateKey = InApplyDamageSpecKey;
		candidateKey.EquipmentType = EEquipmentType::All;
		candidateKey.ActionType = EActionType::All;
		candidateKey.ActionIndex = INDEX_NONE;
		OutApplyDamageSpecKeys.Add(candidateKey);
	}

	// 5) 'Attachment + Equipment + Action + Index Any' Key (All Any Key)
	{
		FApplyDamageSpecKey candidateKey = InApplyDamageSpecKey;
		candidateKey.AttachmentType = EAttachmentType::All;
		candidateKey.EquipmentType = EEquipmentType::All;
		candidateKey.ActionType = EActionType::All;
		candidateKey.ActionIndex = INDEX_NONE;
		OutApplyDamageSpecKeys.Add(candidateKey);
	}
}

UCReaction* UCReactionComponent::CreateReaction(const TSubclassOf<class UCReaction> InSubClass)
{
	UClass* keyClass = InSubClass.Get();
	if (!IsValid(keyClass)) return nullptr;

	UCReaction* newReaction = NewObject<UCReaction>(this, keyClass);
	if (!IsValid(newReaction)) return nullptr;

	newReaction->InitializeReaction(OwnerCharacter_Cached, this);
	ReactionExcutorMap.Add(InSubClass, newReaction);

	return newReaction;
}

UCReaction* UCReactionComponent::FindReaction(const UClass* InClass)
{
	UCReaction** found = ReactionExcutorMap.Find(InClass);
	if (!found) return nullptr;

	UCReaction* foundReaction = *found;

	if (!IsValid(foundReaction))
	{
		// Remove Invalid Entry
		ReactionExcutorMap.Remove(InClass);

		return nullptr;
	}

	return foundReaction;
}

void UCReactionComponent::PrintReactionDataMapInfo() const
{
	FLog::Log(TEXT("==== ReactionDataMap Info ====="));

	const int32 count = ReactionDataMap.Num();
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("Count"), count));

	if (count <= 0)
	{
		FLog::Log(TEXT("[Is Empty]"));
		FLog::Log(TEXT("================================="));
		return;
	}

	FLog::Log(TEXT("=========== Pair Info ==========="));

	int32 index = 0;
	for (const TPair<FReactionDataKey, FReactionData>& pair : ReactionDataMap)
	{
		const FReactionDataKey& key = pair.Key;
		const FReactionData& value = pair.Value;

		FLog::Log(FString::Printf(TEXT("[%s: %d]"), TEXT("PairIndex"), index++));

		PrintReactionDataKeyInfo(key);
		PrintReactionDataInfo(value);
		FLog::Log(TEXT("================================="));
	}
}

void UCReactionComponent::PrintApplyDamageSpecKeyInfo(const FApplyDamageSpecKey& InApplyDamageSpecKey) const
{
	const FString actionIndexText = (InApplyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(InApplyDamageSpecKey.ActionIndex);

	FLog::Log(TEXT("---- ApplyDamageSpecKey Info ----"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AttachmentType"), *UEnum::GetValueAsString(InApplyDamageSpecKey.AttachmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("EquipmentType"), *UEnum::GetValueAsString(InApplyDamageSpecKey.EquipmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(InApplyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReactionComponent::PrintReactionDataKeyInfo(const FReactionDataKey& InReactionDataKey) const
{
	const FApplyDamageSpecKey& applyDamageSpecKey = InReactionDataKey.ApplyDamageSpecKey;
	const FString actionIndexText = (applyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(applyDamageSpecKey.ActionIndex);

	FLog::Log(TEXT("------- ReactionDataKey Info --------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AttachmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.AttachmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("EquipmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.EquipmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(applyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(InReactionDataKey.ReactionType)));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReactionComponent::PrintReactionDataInfo(const FReactionData& InReactionData) const
{
	const FApplyDamageSpecKey& applyDamageSpecKey = InReactionData.ReactionDataKey.ApplyDamageSpecKey;
	const FString actionIndexText = (applyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(applyDamageSpecKey.ActionIndex);

	FLog::Log(TEXT("------ ReactionData Info --------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AttachmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.AttachmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("EquipmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.EquipmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(applyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(InReactionData.ReactionDataKey.ReactionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Montage"), *GetNameSafe(InReactionData.Montage)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("PlayRate"), InReactionData.PlayRate));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("bCanMove"), InReactionData.bCanMove ? 1 : 0));
	FLog::Log(TEXT("---------------------------------"));
}