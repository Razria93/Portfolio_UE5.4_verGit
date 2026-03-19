#include "Component/CReactionComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CStateComponent.h"
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

	MovementComp_Cached = Cast<UCMovementComponent>(OwnerCharacter_Cached->GetComponentByClass(UCMovementComponent::StaticClass()));
	check(MovementComp_Cached);

	StateComp_Cached = Cast<UCStateComponent>(OwnerCharacter_Cached->GetComponentByClass(UCStateComponent::StaticClass()));
	check(StateComp_Cached);

	// Rebuild All
	BuildReactionDataMap(true);
	BuildReactionExecutorMap(true);

	// [Debug] DataMap
	// PrintReactionDataMap();
}

void UCReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UCReactionComponent::HasPendingReactionContext() const
{
	return PendingReactionContext_Cached.IsValidMinimal();
}

bool UCReactionComponent::HasActiveReactionContext() const
{
	return ActiveReactionContext_Cached.IsValidMinimal();
}

bool UCReactionComponent::IsReacting() const
{
	return HasPendingReactionContext() || HasActiveReactionContext();
}

bool UCReactionComponent::TryRequestPendingReaction(const FTakeDamageResult& InTakeDamageResult)
{
	FReactionContext newReactionContext; // OutParameter
	if (!TryBuildReactionContext(InTakeDamageResult, newReactionContext)) return false;

	// Case01. Invalid pending reaction
	if (!HasPendingReactionContext())
	{
		PendingReactionContext_Cached = newReactionContext;
		return true;
	}
	
	if (!QueryReplaceReaction(
		PendingReactionContext_Cached.ReactionExecutor, newReactionContext.ReactionExecutor,
		PendingReactionContext_Cached.ReactionData, newReactionContext.ReactionData))
	{
		return false;
	}

	// Case02. Valid pending reaction
	// Replace current pending with new pending
	PendingReactionContext_Cached = newReactionContext;
	return true;
}

bool UCReactionComponent::TryConsumePendingReaction(FReactionContext& OutReactionContext)
{
	OutReactionContext = FReactionContext();

	// [Guard clause]
	// pending may already have been cleared by a prior consume or tree re-evaluation.
	if (!HasPendingReactionContext()) return false;

	OutReactionContext = PendingReactionContext_Cached;
	PendingReactionContext_Cached = FReactionContext();
	return true;
}

bool UCReactionComponent::TryExecuteReaction(const FReactionContext& InReactionContext)
{
	if (!InReactionContext.IsValidMinimal()) return false;

	UCReaction* newReactionExecutor = InReactionContext.ReactionExecutor;
	const FReactionData& newReactionData = InReactionContext.ReactionData;

	// Clear active reaction context for switching
	if (HasActiveReactionContext())
	{
		// Compare 'current active vs new active'
		if (!QueryReplaceReaction(
			ActiveReactionContext_Cached.ReactionExecutor, newReactionExecutor,
			ActiveReactionContext_Cached.ReactionData, newReactionData))
		{
			return false;
		}

		if (IsValid(ActiveReactionContext_Cached.ReactionExecutor))
		{
			// [Interrupted Flow]
			// Stop() -> OnReactionEnd() -> FinishReaction() -> ClearActiveReaction()
			ActiveReactionContext_Cached.ReactionExecutor->Stop(EReactionStopReason::Interrupted);
		}
		else // Fallback
		{
			FLog::Log(TEXT("[TryExecuteReaction] ActiveReactionContext is valid but executor is invalid. Clear stale active reaction."));
			ClearActiveReaction();
		}
	}

	// Set new reaction state
	UpdateMovementToImmovable(newReactionData);
	UpdateStateToReaction();

	// Execute Failed
	if (!newReactionExecutor->Begin(newReactionData))
	{
		// Clear new reaction
		UpdateMovementToMovable(newReactionData);
		UpdateStateToIdle();
		return false;
	}

	// Execute Success
	ChangeActiveReaction(InReactionContext);
	return true;
}

void UCReactionComponent::FinishReaction()
{
	if (!HasActiveReactionContext()) return;

	UpdateMovementToMovable(ActiveReactionContext_Cached.ReactionData);
	UpdateStateToIdle();
	ClearActiveReaction();
}

void UCReactionComponent::OnReactionBegin()
{
	// TODO:
}

void UCReactionComponent::OnReactionEnd(const UCReaction* InReaction, bool bInterrupted) // Non-Used bInterrupted
{
	if (!HasActiveReactionContext()) return;
	if (!IsValid(InReaction)) return;

	// Stale callback guard
	if (InReaction != ActiveReactionContext_Cached.ReactionExecutor) return;

	FinishReaction();
}

void UCReactionComponent::OnReactionWindowBegin(EReactionWindowType InReactionWindowType, UAnimSequenceBase* Animation)
{
	if (InReactionWindowType == EReactionWindowType::None) return;

	switch (InReactionWindowType)
	{
	case EReactionWindowType::Interruptible:
		if (IsValid(ActiveReactionContext_Cached.ReactionExecutor))
			ActiveReactionContext_Cached.ReactionExecutor->SetInterruptible(true);
		break;

	case EReactionWindowType::Cancelable:
		if (IsValid(ActiveReactionContext_Cached.ReactionExecutor))
			ActiveReactionContext_Cached.ReactionExecutor->SetCancelable(true);
		break;

	case EReactionWindowType::ImmuneToReaction:
		if (IsValid(ActiveReactionContext_Cached.ReactionExecutor))
		{
			ActiveReactionContext_Cached.ReactionExecutor->SetInterruptible(false);
			ActiveReactionContext_Cached.ReactionExecutor->SetCancelable(false);
		}
		break;

	default:
		break;
	}
}

void UCReactionComponent::OnReactionWindowEnd(EReactionWindowType InReactionWindowType, UAnimSequenceBase* Animation)
{
	if (InReactionWindowType == EReactionWindowType::None) return;

	switch (InReactionWindowType)
	{
	case EReactionWindowType::Interruptible:
		if (IsValid(ActiveReactionContext_Cached.ReactionExecutor))
			ActiveReactionContext_Cached.ReactionExecutor->SetInterruptible(false);
		break;

	case EReactionWindowType::Cancelable:
		if (IsValid(ActiveReactionContext_Cached.ReactionExecutor))
			ActiveReactionContext_Cached.ReactionExecutor->SetCancelable(false);
		break;

	case EReactionWindowType::ImmuneToReaction:
		break;

	default:
		break;
	}
}

bool UCReactionComponent::TryBuildReactionContext(const FTakeDamageResult& InTakeDamageResult, FReactionContext& OutReactionContext)
{
	OutReactionContext = FReactionContext();

	if (!ValidateRequest(InTakeDamageResult)) return false;

	const EReactionType newReactionType = ResolveReactionType(InTakeDamageResult);
	if (newReactionType == EReactionType::None) return false;

	FReactionData newReactionData; // OutParameter
	if (!ResolveReactionData(InTakeDamageResult.ApplyDamageSpecKey, newReactionType, newReactionData)) return false;

	UCReaction* newReactionExecutor = ResolveReactionExecutor(newReactionData);
	if (!IsValid(newReactionExecutor)) return false;

	OutReactionContext.ReactionData = newReactionData;
	OutReactionContext.ReactionExecutor = newReactionExecutor;

	return true;
}

bool UCReactionComponent::ValidateRequest(const FTakeDamageResult& takeDamageResult) const
{
	if (!IsValid(OwnerCharacter_Cached)) return false;
	if (!takeDamageResult.bAccepted) return false;

	return true;
}

EReactionType UCReactionComponent::ResolveReactionType(const FTakeDamageResult& takeDamageResult)
{
	// Handled by CTakenDamageComponent::BuildResult()
	if (takeDamageResult.bTriggerDeathReaction) return EReactionType::Dead;
	if (takeDamageResult.bTriggerHitReaction) return EReactionType::Hit;

	return EReactionType::None;
}

bool UCReactionComponent::ResolveReactionData(const FApplyDamageSpecKey& InApplyDamageSpecKey, EReactionType InReactionType, FReactionData& OutReactionData)
{
	OutReactionData = FReactionData();

	TArray<FApplyDamageSpecKey> candidateKeys; // OutParameter
	BuildCandidateSpecKeys(InApplyDamageSpecKey, candidateKeys);

	for (const FApplyDamageSpecKey& candidateKey : candidateKeys)
	{
		FReactionDataKey reactionDataKey;
		reactionDataKey.ApplyDamageSpecKey = candidateKey;	// ApplyDamage Part Condition
		reactionDataKey.ReactionType = InReactionType;		// ReactionType

		// Find ReactionData
		const FReactionData* found = ReactionDataMap.Find(reactionDataKey);
		if (!found) continue;

		const FReactionData& reactionData = *found;

		// [Debug] ReactionData
		// PrintReactionDataInfo(reactionData);

		OutReactionData = reactionData;

		return true;
	}

	return false;
}

UCReaction* UCReactionComponent::ResolveReactionExecutor(const FReactionData& InReactionData)
{
	// 1) Try reuse cached Reaction; return if valid
	UCReaction* foundReaction = FindReactionExecutor(InReactionData.ReactionExecutorKey.Get());
	if (IsValid(foundReaction)) return foundReaction;

	// 2) [Policy] Try Add and cache Reaction; return if valid
	UCReaction* addReaction = AddReactionExecutor(InReactionData.ReactionExecutorKey);
	if (IsValid(addReaction)) return addReaction;

	// [Debug] ReactionData is Valid; but Find and Add Failed
	return nullptr;
}

bool UCReactionComponent::QueryReplaceReaction(UCReaction* InCurrentReactionExecutor, UCReaction* InIncomingReactionExecutor, const FReactionData& InCurrentReactionData, const FReactionData& InIncomingReactionData)
{
	if (!InIncomingReactionData.IsValidMinimal()) return false;
	if (!IsValid(InIncomingReactionExecutor)) return false;

	if (!InCurrentReactionData.IsValidMinimal()) return true;
	if (!IsValid(InCurrentReactionExecutor)) return false;

	FReactionQueryContext reactionQueryContext = FReactionQueryContext();
	reactionQueryContext.CurrentReactionExecutor = InCurrentReactionExecutor;
	reactionQueryContext.IncomingReactionExecutor = InIncomingReactionExecutor;
	reactionQueryContext.CurrentReactionData = InCurrentReactionData;
	reactionQueryContext.IncomingReactionData = InIncomingReactionData;

	// [POLICY] Duplicate ReactionDataKey: Currently set to 'restart'.
	// (Options: ignore | restart | stop-then-play)
	if (reactionQueryContext.CurrentReactionData.ReactionDataKey == reactionQueryContext.IncomingReactionData.ReactionDataKey)
	{
		FLog::Log(FString::Printf(
			TEXT("[QueryReplaceReaction] Duplicate key (Current = %s, Incoming = %s)"),
			*GetNameSafe(reactionQueryContext.CurrentReactionExecutor),
			*GetNameSafe(reactionQueryContext.IncomingReactionExecutor)));
	}

	if (reactionQueryContext.CurrentReactionData.Priority < reactionQueryContext.IncomingReactionData.Priority)
	{
		FLog::Log(FString::Printf(
			TEXT("[RejectIncomingReaction] Lower priority (Current = %s, Incoming = %s, CurrentPriority = %d, IncomingPriority = %d)"),
			*GetNameSafe(reactionQueryContext.CurrentReactionExecutor),
			*GetNameSafe(reactionQueryContext.IncomingReactionExecutor),
			reactionQueryContext.CurrentReactionData.Priority,
			reactionQueryContext.IncomingReactionData.Priority));
		return false;
	}

	if (!reactionQueryContext.CurrentReactionExecutor->AllowInterruptionBy(reactionQueryContext))
	{
		FLog::Log(FString::Printf(
			TEXT("[RejectIncomingReaction] Not interruptible (Current = %s, Incoming = %s)"),
			*GetNameSafe(reactionQueryContext.CurrentReactionExecutor),
			*GetNameSafe(reactionQueryContext.IncomingReactionExecutor)));
		return false;
	}

	if (!reactionQueryContext.IncomingReactionExecutor->WantToInterrupt(reactionQueryContext))
	{
		FLog::Log(FString::Printf(
			TEXT("[RejectIncomingReaction] Incoming cannot interrupt now (Incoming = %s)"),
			*GetNameSafe(reactionQueryContext.IncomingReactionExecutor)));
		return false;
	}

	return true;
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
				// [Debug] Duplicate key: Override data
				FLog::Log(TEXT("[Duplicate key] Overwrite Value"));
				ReactionDataMap[reactionDataKey] = reactionData;
			}
			else // bRebuildAll == false
			{
				// [Policy] Currently set to 'skip'. (Options: ignore | restart | stop-then-play)
				continue;
			}
		}
		else // Contains(reactionDataKey) == false
		{
			ReactionDataMap.Add(reactionDataKey, reactionData);
		}
	}
}

void UCReactionComponent::BuildReactionExecutorMap(bool bRebuildAll)
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
			const UCReaction* found = FindReactionExecutor(reactionExcutorKey);
			if (IsValid(found)) continue;
		}

		// 2) Add cached Reaction
		UCReaction* add = AddReactionExecutor(reactionClass);
		if (!IsValid(add))
		{
			FLog::Log(FString::Printf(TEXT("[BuildReactionExecutorMap] Failed to add ReactionExecutor. ReactionClass = %s"), *GetNameSafe(reactionClass.Get())));
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

UCReaction* UCReactionComponent::AddReactionExecutor(const TSubclassOf<class UCReaction> InSubClass)
{
	UClass* keyClass = InSubClass.Get();
	if (!IsValid(keyClass)) return nullptr;

	UCReaction* addReactionExecutor = NewObject<UCReaction>(this, InSubClass);
	if (!IsValid(addReactionExecutor)) return nullptr;

	addReactionExecutor->Initialize(OwnerCharacter_Cached, this);
	ReactionExcutorMap.Add(keyClass, addReactionExecutor);

	return addReactionExecutor;
}

UCReaction* UCReactionComponent::FindReactionExecutor(const UClass* InClass)
{
	UCReaction** found = ReactionExcutorMap.Find(InClass);
	if (!found) return nullptr;

	UCReaction* foundReactionExecutor = *found;

	if (!IsValid(foundReactionExecutor))
	{
		// Remove Invalid Entry
		ReactionExcutorMap.Remove(InClass);

		return nullptr;
	}

	return foundReactionExecutor;
}

void UCReactionComponent::ChangeActiveReaction(const FReactionContext& InReactionContext)
{
	if (!InReactionContext.IsValidMinimal()) return;

	EReactionType prevReactionType = ActiveReactionType_Cached;
	ActiveReactionType_Cached = InReactionContext.ReactionData.ReactionDataKey.ReactionType;

	ActiveReactionContext_Cached = InReactionContext;

	if (OnReactionTypeChanged.IsBound())
		OnReactionTypeChanged.Broadcast(OwnerCharacter_Cached, prevReactionType, ActiveReactionType_Cached);
}

void UCReactionComponent::ClearActiveReaction()
{
	EReactionType prevReactionType = ActiveReactionType_Cached;
	ActiveReactionType_Cached = EReactionType::None;

	ActiveReactionContext_Cached = FReactionContext();

	if (OnReactionTypeChanged.IsBound())
		OnReactionTypeChanged.Broadcast(OwnerCharacter_Cached, prevReactionType, ActiveReactionType_Cached);
}


void UCReactionComponent::UpdateMovementToImmovable(const FReactionData& InReactionData)
{
	if (!IsValid(MovementComp_Cached)) return;

	if (InReactionData.bCanMove == false)
	{
		MovementComp_Cached->SetStop();
	}
}

void UCReactionComponent::UpdateMovementToMovable(const FReactionData& InReactionData)
{
	if (!IsValid(MovementComp_Cached)) return;

	if (InReactionData.bCanMove == false)
	{
		MovementComp_Cached->SetMove();
	}
}

void UCReactionComponent::UpdateStateToReaction()
{
	if (!IsValid(StateComp_Cached)) return;

	StateComp_Cached->SetReactionMode();
}

void UCReactionComponent::UpdateStateToIdle()
{
	if (!IsValid(StateComp_Cached)) return;

	StateComp_Cached->SetIdleMode();
}

void UCReactionComponent::PrintReactionInfoSummary() const
{
	FLog::Log(TEXT("=== Reaction Intergrated Info ==="));

	// Component State
	PrintComponentStateInfo();

	// Active ReactionData
	{
		FLog::Log(TEXT("=== Active ReactionData Info ===="));

		if (!ActiveReactionContext_Cached.ReactionData.IsValidMinimal())
		{
			FLog::Log(TEXT("[ActiveReactionData] Invalid / Empty"));
		}
		else
		{
			PrintReactionDataInfo(ActiveReactionContext_Cached.ReactionData);
		}
	}

	// Active Executor Runtime
	{
		FLog::Log(TEXT("== Active ReactionExcutor Info =="));

		if (!IsValid(ActiveReactionContext_Cached.ReactionExecutor))
		{
			FLog::Log(TEXT("[ActiveExecutor] None"));
		}
		else
		{
			PrintReactionExcutorInfo(ActiveReactionContext_Cached.ReactionExecutor);
			PrintReactionExecutorRuntimeInfo(ActiveReactionContext_Cached.ReactionExecutor);
		}
	}

	FLog::Log(TEXT("================================="));
}

void UCReactionComponent::PrintReactionDataMap() const
{
	FLog::Log(TEXT("===== ReactionDataMap Info ======"));

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

void UCReactionComponent::PrintComponentStateInfo() const
{
	FLog::Log(TEXT("-------- Component State --------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("HasPendingReactionContext"), HasPendingReactionContext() ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("HasActiveReactionContext"), HasActiveReactionContext() ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(ActiveReactionType_Cached)));
	FLog::Log(TEXT("---------------------------------"));
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

	FLog::Log(TEXT("----- ReactionDataKey Info ------"));
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
	// ApplyDamageSpec Key
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AttachmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.AttachmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("EquipmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.EquipmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(applyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));

	// ReactionType Key
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(InReactionData.ReactionDataKey.ReactionType)));

	// RactionExecutor Key
	UClass* executorClass = InReactionData.ReactionExecutorKey.Get();
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ExecutorKey"), *GetNameSafe(executorClass)));

	// Raction Montage Data
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Montage"), *GetNameSafe(InReactionData.Montage)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("PlayRate"), InReactionData.PlayRate));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("bCanMove"), InReactionData.bCanMove ? 1 : 0));
	FLog::Log(TEXT("---------------------------------"));

}

void UCReactionComponent::PrintReactionExcutorInfo(const UCReaction* InReaction) const
{
	FLog::Log(TEXT("----- ReactionExcutor Info ------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ExecutorObject"), *GetNameSafe(InReaction)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ExecutorClass"), *GetNameSafe(InReaction->GetClass())));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReactionComponent::PrintReactionExecutorRuntimeInfo(const UCReaction* InReaction) const
{
	if (!IsValid(InReaction)) return;
	InReaction->PrintReactionExecutorRuntimeInfo_Public();
}