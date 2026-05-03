#include "Component/CReactionComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CHealthComponent.h"
#include "Reaction/CReaction.h"

#include "Type/CWeaponStructure.h"

UCReactionComponent::UCReactionComponent()
{
}

void UCReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	MovementComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCMovementComponent>();
	check(MovementComp_Cached);

	StateComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCStateComponent>();
	check(StateComp_Cached);

	ActionComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCActionComponent>();
	check(ActionComp_Cached);

	HealthComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCHealthComponent>();
	check(HealthComp_Cached);

	// Rebuild All
	BuildReactionDataMap(true);
	BuildReactionExecutorMap(true);

	// [Debug] DataMap
	// PrintReactionDataMap();
}

bool UCReactionComponent::IsActiveReaction() const
{
	return ActiveReactionType_Cached != EReactionType::None
		&& ActiveReactionType_Cached != EReactionType::Max;
}

bool UCReactionComponent::ApplyReactionDecision(const FReactionOrchestrationResult& InReactionOrchestrationResult)
{
	switch (InReactionOrchestrationResult.Decision)
	{
	case EReactionOrchestrationDecision::Start:
		return StartReaction(InReactionOrchestrationResult.ReactionContext);

	case EReactionOrchestrationDecision::Interrupt:
		return InterruptReaction(InReactionOrchestrationResult.ReactionContext);

	case EReactionOrchestrationDecision::Ignore:
	case EReactionOrchestrationDecision::Reject:
	case EReactionOrchestrationDecision::None:
	default:
		return false;
	}
}

bool UCReactionComponent::StartReaction(const FReactionContext& InReactionContext)
{
	return ReplaceActiveReaction(InReactionContext, EReactionStopReason::Started);
}

bool UCReactionComponent::InterruptReaction(const FReactionContext& InReactionContext)
{
	return ReplaceActiveReaction(InReactionContext, EReactionStopReason::Interrupted);
}

void UCReactionComponent::EndReaction()
{
	EndActiveReactionInternal();
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

void UCReactionComponent::OnReactionBegin()
{
}

void UCReactionComponent::OnReactionEnd(const UCReaction* InReaction, bool bInterrupted) // Non-Used bInterrupted
{
	if (!IsActiveReaction()) return;
	if (!IsValid(InReaction)) return;

	// Stale callback guard
	if (InReaction != ActiveReactionContext_Cached.ReactionExecutor) return;

	EndReaction();
}

void UCReactionComponent::OnReactionWindowBegin(EReactionWindowType InReactionWindowType, UAnimSequenceBase* Animation)
{
	if (InReactionWindowType == EReactionWindowType::None) return;
	if (!IsActiveReaction()) return;

	UCReaction* activeExecutor = ActiveReactionContext_Cached.ReactionExecutor;
	if (!IsValid(activeExecutor)) return;

	switch (InReactionWindowType)
	{
	case EReactionWindowType::Interruptible:
		activeExecutor->SetInterruptible(true);
		break;

	case EReactionWindowType::Cancelable:
		activeExecutor->SetCancelable(true);
		break;

	case EReactionWindowType::ImmuneToReaction:
		activeExecutor->SetInterruptible(false);
		activeExecutor->SetCancelable(false);
		break;

	default:
		break;
	}
}

void UCReactionComponent::OnReactionWindowEnd(EReactionWindowType InReactionWindowType, UAnimSequenceBase* Animation)
{
	if (InReactionWindowType == EReactionWindowType::None) return;
	if (!IsActiveReaction()) return;

	UCReaction* activeExecutor = ActiveReactionContext_Cached.ReactionExecutor;
	if (!IsValid(activeExecutor)) return;

	switch (InReactionWindowType)
	{
	case EReactionWindowType::Interruptible:
	{
		activeExecutor->SetInterruptible(false);
		break;
	}

	case EReactionWindowType::Cancelable:
	{
		activeExecutor->SetCancelable(false);
		break;
	}

	case EReactionWindowType::ImmuneToReaction:
		break;

	default:
		break;
	}
}

bool UCReactionComponent::ReplaceActiveReaction(const FReactionContext& InContext, EReactionStopReason InStopReason)
{
	if (!InContext.IsValidMinimal()) return false;

	StopActiveReactionInternal(InStopReason);
	EndActiveReactionInternal();

	return StartActiveReactionInternal(InContext);
}

bool UCReactionComponent::StartActiveReactionInternal(const FReactionContext& InReactionContext)
{
	if (!InReactionContext.IsValidMinimal()) return false;

	UCReaction* reactionExecutor = InReactionContext.ReactionExecutor;
	if (!IsValid(reactionExecutor)) return false;

	const FReactionData& reactionData = InReactionContext.ReactionData;

	AbortActiveActionForReaction();
	EnterReactionState(reactionData);

	if (!reactionExecutor->Begin(reactionData))
	{
		ExitReactionState(reactionData);
		return false;
	}

	SetActiveReaction(InReactionContext);
	return true;
}

void UCReactionComponent::StopActiveReactionInternal(EReactionStopReason InStopReason)
{
	if (!IsActiveReaction()) return;

	UCReaction* activeExecutor = ActiveReactionContext_Cached.ReactionExecutor;

	if (IsValid(activeExecutor))
	{
		activeExecutor->Stop(InStopReason);
	}
}

void UCReactionComponent::EndActiveReactionInternal()
{
	if (!IsActiveReaction()) return;

	const FReactionData activeReactionData = ActiveReactionContext_Cached.ReactionData;

	if (activeReactionData.IsValidMinimal())
	{
		ExitReactionState(activeReactionData);
	}

	ClearActiveReaction();
}

void UCReactionComponent::SetActiveReaction(const FReactionContext& InReactionContext)
{
	if (!InReactionContext.IsValidMinimal()) return;

	const EReactionType prevReactionType = ActiveReactionType_Cached;

	ActiveReactionType_Cached = InReactionContext.ReactionData.ReactionDataKey.ReactionType;
	ActiveReactionContext_Cached = InReactionContext;

	if (OnReactionTypeChanged.IsBound())
	{
		OnReactionTypeChanged.Broadcast(OwnerCharacter_Cached, prevReactionType, ActiveReactionType_Cached);
	}
}

void UCReactionComponent::ClearActiveReaction()
{
	EReactionType prevReactionType = ActiveReactionType_Cached;

	ActiveReactionType_Cached = EReactionType::None;
	ActiveReactionContext_Cached = FReactionContext();

	if (OnReactionTypeChanged.IsBound())
	{
		OnReactionTypeChanged.Broadcast(OwnerCharacter_Cached, prevReactionType, ActiveReactionType_Cached);
	}
}

void UCReactionComponent::EnterReactionState(const FReactionData& InReactionData)
{
	if (IsValid(MovementComp_Cached) && !InReactionData.bCanMove)
	{
		MovementComp_Cached->SetStop();
	}

	if (IsValid(StateComp_Cached))
	{
		StateComp_Cached->SetReactionState();
	}
}

void UCReactionComponent::ExitReactionState(const FReactionData& InReactionData)
{
	const bool bAlive = IsValid(HealthComp_Cached) && HealthComp_Cached->IsAlive();
	const bool bDeadExecution = IsValid(StateComp_Cached) && StateComp_Cached->GetCurrentExecutionState() == EExecutionState::Dead;

	if (!bAlive || bDeadExecution) return;

	if (IsValid(MovementComp_Cached) && !InReactionData.bCanMove)
	{
		MovementComp_Cached->SetMove();
	}

	if (IsValid(StateComp_Cached))
	{
		StateComp_Cached->SetIdleState();
	}
}

void UCReactionComponent::AbortActiveActionForReaction()
{
	if (!IsValid(ActionComp_Cached)) return;

	if (ActionComp_Cached->GetCurrentActionType() != EActionType::Idle)
	{
		ActionComp_Cached->AbortCurrentAction(EActionAbortReason::Reaction);
	}
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

	// 1) Exact: Weapon + Action + Index
	OutApplyDamageSpecKeys.Add(InApplyDamageSpecKey);

	// 2) Any Index: Weapon + Action + AnyIndex
	{
		FApplyDamageSpecKey candidateKey = InApplyDamageSpecKey;
		candidateKey.ActionIndex = INDEX_NONE;
		OutApplyDamageSpecKeys.Add(candidateKey);
	}

	// 3) Any Action: Weapon + AnyAction + AnyIndex
	{
		FApplyDamageSpecKey candidateKey = InApplyDamageSpecKey;
		candidateKey.ActionType = EActionType::All;
		candidateKey.ActionIndex = INDEX_NONE;
		OutApplyDamageSpecKeys.Add(candidateKey);
	}

	// 4) Any Weapon: AnyWeapon + AnyAction + AnyIndex
	{
		FApplyDamageSpecKey candidateKey = InApplyDamageSpecKey;
		candidateKey.WeaponType = EWeaponType::All;
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
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("IsActiveReaction"), IsActiveReaction() ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(ActiveReactionType_Cached)));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReactionComponent::PrintApplyDamageSpecKeyInfo(const FApplyDamageSpecKey& InApplyDamageSpecKey) const
{
	const FString actionIndexText = (InApplyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(InApplyDamageSpecKey.ActionIndex);

	FLog::Log(TEXT("---- ApplyDamageSpecKey Info ----"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(InApplyDamageSpecKey.WeaponType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(InApplyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReactionComponent::PrintReactionDataKeyInfo(const FReactionDataKey& InReactionDataKey) const
{
	const FApplyDamageSpecKey& applyDamageSpecKey = InReactionDataKey.ApplyDamageSpecKey;
	const FString actionIndexText = (applyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(applyDamageSpecKey.ActionIndex);

	FLog::Log(TEXT("----- ReactionDataKey Info ------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(applyDamageSpecKey.WeaponType)));
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
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(applyDamageSpecKey.WeaponType)));
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
