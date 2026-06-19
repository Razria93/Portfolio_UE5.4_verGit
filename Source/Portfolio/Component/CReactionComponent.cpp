#include "Component/CReactionComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Reaction/CReaction.h"

#include "Type/CWeaponStructure.h"

UCReactionComponent::UCReactionComponent()
{
}

// Lifecycle

void UCReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	MovementComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCMovementComponent>();
	StateComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCStateComponent>();
	HealthComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCHealthComponent>();
	DefenseComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCDefenseComponent>();
	ActionComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCActionComponent>();
	ObservableOverlayComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCObservableOverlayComponent>();

	// Rebuild All
	BuildReactionDataMap(true);
	BuildReactionExecutorMap(true);

	// Init Reaction State
	ActiveReactionType = EReactionType::Idle;
}

// Query

bool UCReactionComponent::IsActive() const
{
	return ActiveReactionType != EReactionType::None
		&& ActiveReactionType != EReactionType::Idle
		&& ActiveReactionType != EReactionType::All
		&& ActiveReactionType != EReactionType::Max;
}

EReactionType UCReactionComponent::GetActiveReactionType() const
{
	return ActiveReactionType;
}

bool UCReactionComponent::GetActiveReactionData(FReactionData& OutData) const
{
	OutData = FReactionData();

	if (!IsActive()) return false;
	if (!ActiveReactionData.IsValidMinimal()) return false;

	OutData = ActiveReactionData;
	return true;
}

UCReaction* UCReactionComponent::GetActiveReactionExecutor() const
{
	if (!IsActive()) return nullptr;
	if (!IsValid(ActiveReactionExecutor)) return nullptr;

	return ActiveReactionExecutor;
}

// Data Resolve

bool UCReactionComponent::ResolveReactionData(const FReactionDataKey& InDataKey, FReactionData& OutData)
{
	OutData = FReactionData();

	if (!InDataKey.IsValidMinimal()) return false;

	TArray<FApplyDamageSpecKey> candidateKeys; // OutParameter
	EReactionType reactionType = InDataKey.ReactionType;
	
	// Candidate SpecKey
	BuildCandidateSpecKeys(InDataKey.ApplyDamageSpecKey, candidateKeys);

	for (const FApplyDamageSpecKey& candidateKey : candidateKeys)
	{
		FReactionDataKey reactionDataKey;

		// Rebuild CandidateSpecKey + Type
		reactionDataKey.ApplyDamageSpecKey = candidateKey;
		reactionDataKey.ReactionType = reactionType;

		// Find ReactionData
		const FReactionData* foundPtr = ReactionDataMap.Find(reactionDataKey);
		if (!foundPtr) continue;

		const FReactionData& found = *foundPtr;
		if (!found.IsValidMinimal()) continue;

		// [Debug] ReactionData
		// PrintReactionDataInfo(found);

		OutData = found;
		return true;
	}

	return false;
}

UCReaction* UCReactionComponent::ResolveReactionExecutor(const FReactionData& InData)
{
	// 1) Try reuse cached Reaction; return if valid
	UCReaction* found = FindReactionExecutor(InData.ReactionExecutorKey.Get());
	if (IsValid(found)) return found;

	// 2) [Policy] Try Add and cache Reaction; return if valid
	UCReaction* add = AddReactionExecutor(InData.ReactionExecutorKey);
	if (IsValid(add)) return add;

	// [Debug] ReactionData is Valid; but Find and Add Failed
	return nullptr;
}

// Execution Entry

bool UCReactionComponent::ApplyReactionDecision(const FReactionExecutionResult& InResult)
{
	if (!IsValid(OwnerCharacter_Cached)) return false;
	if (!InResult.IsAcceptedDecision()) return false;

	FLog::Log(FString::Printf(
		TEXT("[ReactionDecision] ApplyMode=%s | ReactionType=%s"),
		*UEnum::GetValueAsString(InResult.ApplyMode),
		*UEnum::GetValueAsString(InResult.ResolvedContext.ReactionDataKey.ReactionType)));

	switch (InResult.ApplyMode)
	{
	case EExecutionApplyMode::Start:
	{
		FLog::Log(TEXT("[Start]"));

		if (!ApplyObservableOverlayHandlings(InResult.OverlayHandlings))
		{
			FLog::Log(TEXT("[ReactionDecision] Overlay handling failed."));
			return false;
		}

		const bool bStarted = StartReaction(InResult.ResolvedContext);
		if (!bStarted)
		{
			FLog::Log(TEXT("[ReactionDecision] Start reaction failed."));
		}
		return bStarted;
	}

	case EExecutionApplyMode::Reserve:
	{
		// [NOTE] Reaction does not support reserved execution.
		FLog::Log(TEXT("[ReactionDecision] Reserve is not supported."));
		return false;
	}

	case EExecutionApplyMode::Intervene:
	{
		FLog::Log(TEXT("[Intervene]"));

		// [NOTE] Try Apply Intervention
		if (!ApplyExecutionInterventionDirective(InResult.InterventionDirective))
		{
			FLog::Log(TEXT("[ReactionDecision] Intervention failed."));
			return false;
		}
		if (!ApplyObservableOverlayHandlings(InResult.OverlayHandlings))
		{
			FLog::Log(TEXT("[ReactionDecision] Overlay handling failed."));
			return false;
		}

		const bool bStarted = StartReaction(InResult.ResolvedContext);
		if (!bStarted)
		{
			FLog::Log(TEXT("[ReactionDecision] Start reaction failed."));
		}
		return bStarted;
	}
	
	default:
		FLog::Log(TEXT("[ReactionDecision] Invalid apply mode."));
		return false;
	}
}

bool UCReactionComponent::RequestInterruptActiveReaction(const FExecutionInterventionDirective& InDirective)
{
	if (!InDirective.IsValidRequest()) return false;
	if (InDirective.TargetDomain != EExecutionDomain::Reaction) return false;

	return InterruptActiveReaction(InDirective);
}

// Execution Result Hooks

void UCReactionComponent::HandleApplyReactionFinished(const UCReaction* InReaction, EReactionFinishReason InFinishReason)
{
	if (!IsActive()) return;
	if (!IsValid(InReaction)) return;
	if (InReaction != GetActiveReactionExecutor()) return;

	EndActiveReaction(InFinishReason);
}

// Cross-System Dispatch

void UCReactionComponent::RequestConsumeDeferredAction(EDeferredActionConsumeKey InConsumeKey)
{
	if (!IsValid(ActionComp_Cached)) return;

	ActionComp_Cached->ConsumeDeferredAction(InConsumeKey);
}

// Notify Routing

void UCReactionComponent::HandleReactionNotifyCommand(EReactionNotifyCommand InNotifyCommand)
{
	if (InNotifyCommand == EReactionNotifyCommand::None || InNotifyCommand == EReactionNotifyCommand::Max) return;

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->HandleNotifyCommand(InNotifyCommand);
}

void UCReactionComponent::HandleReactionAllowInterventionWindowBegin(FName InWindowKey)
{
	if (InWindowKey.IsNone()) return;

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->OpenAllowInterventionWindow(InWindowKey);
}

void UCReactionComponent::HandleReactionAllowInterventionWindowEnd(FName InWindowKey)
{
	if (InWindowKey.IsNone()) return;

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->CloseAllowInterventionWindow(InWindowKey);
}

void UCReactionComponent::HandleReactionFeedback(FName InTriggerKey)
{
	if (InTriggerKey.IsNone()) return;

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->HandleNotifyFeedback(EReactionFeedbackTiming::TriggerOnce, InTriggerKey);
}

void UCReactionComponent::HandleReactionFeedbackWindowBegin(FName InTriggerKey)
{
	if (InTriggerKey.IsNone()) return;

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->HandleNotifyFeedback(EReactionFeedbackTiming::TriggerWindowBegin, InTriggerKey);
}

void UCReactionComponent::HandleReactionFeedbackWindowEnd(FName InTriggerKey)
{
	if (InTriggerKey.IsNone()) return;

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->HandleNotifyFeedback(EReactionFeedbackTiming::TriggerWindowEnd, InTriggerKey);
}

// Data Build

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
		ReactionExecutorMap.Reset();
	}

	for (const FReactionData& reactionData : ReactionDatas)
	{
		if (!reactionData.IsValidMinimal()) continue;

		UClass* executorkey = reactionData.ReactionExecutorKey.Get();
		if (!IsValid(executorkey)) continue;

		// 1) Find existing cached Reaction
		if (!bRebuildAll)
		{
			const UCReaction* found = FindReactionExecutor(executorkey);
			if (IsValid(found)) continue;
		}

		// 2) Add cached Reaction
		UCReaction* add = AddReactionExecutor(executorkey);
		if (!IsValid(add))
		{
			FLog::Log(FString::Printf(TEXT("[BuildReactionExecutorMap] Failed to add ReactionExecutor. ReactionExecutorKey = %s"), *GetNameSafe(reactionData.ReactionExecutorKey.Get())));
			continue;
		}
	}
}

UCReaction* UCReactionComponent::AddReactionExecutor(const TSubclassOf<class UCReaction> InSubClass)
{
	UClass* executorKey = InSubClass.Get();
	if (!IsValid(executorKey)) return nullptr;

	UCReaction* add = NewObject<UCReaction>(this, InSubClass);
	if (!IsValid(add)) return nullptr;

	add->Initialize(OwnerCharacter_Cached, this);
	ReactionExecutorMap.Add(executorKey, add);

	return add;
}

UCReaction* UCReactionComponent::FindReactionExecutor(const UClass* InClass)
{
	UCReaction** foundPtr = ReactionExecutorMap.Find(InClass);
	if (!foundPtr) return nullptr;

	UCReaction* found = *foundPtr;

	if (!IsValid(found))
	{
		// Remove Invalid Entry
		ReactionExecutorMap.Remove(InClass);

		return nullptr;
	}

	return found;
}

// Data Resolve Helpers

void UCReactionComponent::BuildCandidateSpecKeys(const FApplyDamageSpecKey& InSpecKey, TArray<FApplyDamageSpecKey>& OutSpecKeys) const
{
	OutSpecKeys.Reset();

	// 1) Exact: Weapon + Action + Index
	OutSpecKeys.Add(InSpecKey);

	// 2) Any Index: Weapon + Action + AnyIndex
	{
		FApplyDamageSpecKey candidateKey = InSpecKey;
		candidateKey.ActionIndex = INDEX_NONE;
		OutSpecKeys.Add(candidateKey);
	}

	// 3) Any Action: Weapon + AnyAction + AnyIndex
	{
		FApplyDamageSpecKey candidateKey = InSpecKey;
		candidateKey.ActionType = EActionType::All;
		candidateKey.ActionIndex = INDEX_NONE;
		OutSpecKeys.Add(candidateKey);
	}

	// 4) Any Weapon: AnyWeapon + AnyAction + AnyIndex
	{
		FApplyDamageSpecKey candidateKey = InSpecKey;
		candidateKey.WeaponType = EWeaponType::All;
		candidateKey.ActionType = EActionType::All;
		candidateKey.ActionIndex = INDEX_NONE;
		OutSpecKeys.Add(candidateKey);
	}
}

// Decision Apply

bool UCReactionComponent::ApplyExecutionInterventionDirective(const FExecutionInterventionDirective& InDirective)
{
	if (!InDirective.IsRequested()) return true;
	if (!InDirective.IsValidRequest()) return false;

	switch (InDirective.TargetDomain)
	{
	case EExecutionDomain::Action:
		return IsValid(ActionComp_Cached) && ActionComp_Cached->RequestInterruptActiveAction(InDirective);

	case EExecutionDomain::Reaction:
		return InterruptActiveReaction(InDirective);

	default:
		return false;
	}
}

bool UCReactionComponent::ApplyObservableOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings)
{
	if (InHandlings.IsEmpty()) return true;

	return IsValid(ObservableOverlayComp_Cached) && ObservableOverlayComp_Cached->ApplyObservableOverlayHandlings(InHandlings);
}

// Execution Operations

bool UCReactionComponent::StartReaction(const FReactionExecutionContext& InContext)
{
	if (IsActive()) return false;
	if (!InContext.IsValidMinimal()) return false;

	UCReaction* incomingExecutor = InContext.ReactionExecutor;
	if (!IsValid(incomingExecutor)) return false;

	const FReactionData& incomingData = InContext.ReactionData;

	EnterReactionState(incomingData);

	if (!incomingExecutor->Start(incomingData))
	{
		ExitReactionState(incomingData);
		return false;
	}

	SetActiveReactionContext(InContext);
	return true;
}

bool UCReactionComponent::InterruptActiveReaction(const FExecutionInterventionDirective& InDirective)
{
	if (!IsActive()) return true;

	const EReactionFinishReason finishReason = ConvertExecutionStopReasonToReactionFinishReason(InDirective.StopReason);

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor))
	{
		// [NOTE] Fallback when executor Interrupt() did not clear the active state through callback.
		return EndActiveReaction(finishReason);
	}

	activeExecutor->Interrupt(InDirective);

	if (IsActive())
	{
		// [NOTE] Fallback when executor Interrupt() did not clear the active state through callback.
		return EndActiveReaction(finishReason);
	}

	return !IsActive();
}

bool UCReactionComponent::EndActiveReaction(EReactionFinishReason InFinishReason)
{
	if (!IsActive()) return true;

	const FReactionData activeData = ActiveReactionData;

	if (activeData.IsValidMinimal())
	{
		ExitReactionState(activeData);
	}

	ClearActiveReactionContext();

	return !IsActive();
}

// Active Context

void UCReactionComponent::SetActiveReactionContext(const FReactionExecutionContext& InContext)
{
	if (!InContext.IsValidMinimal()) return;

	const EReactionType prevReactionType = ActiveReactionType;

	ActiveReactionType = InContext.ReactionDataKey.ReactionType;
	ActiveReactionData = InContext.ReactionData;
	ActiveReactionExecutor = InContext.ReactionExecutor;

	if (OnReactionTypeChanged.IsBound())
	{
		OnReactionTypeChanged.Broadcast(OwnerCharacter_Cached, prevReactionType, ActiveReactionType);
	}
}

void UCReactionComponent::ClearActiveReactionContext()
{
	const EReactionType prevReactionType = ActiveReactionType;

	ActiveReactionType = EReactionType::None;
	ActiveReactionData = FReactionData();
	ActiveReactionExecutor = nullptr;

	if (OnReactionTypeChanged.IsBound())
	{
		OnReactionTypeChanged.Broadcast(OwnerCharacter_Cached, prevReactionType, ActiveReactionType);
	}
}

// State Transition

void UCReactionComponent::EnterReactionState(const FReactionData& InData)
{
	if (IsValid(MovementComp_Cached) && !InData.bCanMove)
	{
		MovementComp_Cached->SetStop();
	}

	if (IsValid(StateComp_Cached))
	{
		StateComp_Cached->SetReactionState();
	}
}

void UCReactionComponent::ExitReactionState(const FReactionData& InData)
{
	const bool bAlive = IsValid(HealthComp_Cached) && HealthComp_Cached->IsAlive();
	const bool bDeadExecution = IsValid(StateComp_Cached) && StateComp_Cached->GetCurrentExecutionState() == EExecutionState::Dead;

	if (!bAlive || bDeadExecution) return;

	if (IsValid(MovementComp_Cached) && !InData.bCanMove)
	{
		MovementComp_Cached->SetMove();
	}

	if (IsValid(StateComp_Cached))
	{
		StateComp_Cached->SetIdleState();
	}
}

// Conversion

EReactionFinishReason UCReactionComponent::ConvertExecutionStopReasonToReactionFinishReason(EExecutionStopReason InStopReason) const
{
	switch (InStopReason)
	{
	case EExecutionStopReason::Interrupted:
		return EReactionFinishReason::Interrupted;

	default:
		return EReactionFinishReason::Ignored;
	}
}

// Debug

void UCReactionComponent::PrintReactionInfoSummary() const
{
	FLog::Log(TEXT("=== Reaction Intergrated Info ==="));

	// Component State
	PrintComponentStateInfo();

	// Active ReactionData
	{
		FLog::Log(TEXT("=== Active ReactionData Info ===="));

		if (!ActiveReactionData.IsValidMinimal())
		{
			FLog::Log(TEXT("[ActiveReactionData] Invalid / Empty"));
		}
		else
		{
			PrintReactionDataInfo(ActiveReactionData);
		}
	}

	// Active Executor Runtime
	{
		FLog::Log(TEXT("== Active ReactionExcutor Info =="));

		if (!IsValid(ActiveReactionExecutor))
		{
			FLog::Log(TEXT("[ActiveExecutor] None"));
		}
		else
		{
			PrintReactionExcutorInfo(ActiveReactionExecutor);
			PrintReactionExecutorRuntimeInfo(ActiveReactionExecutor);
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
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("IsActive"), IsActive() ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(ActiveReactionType)));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReactionComponent::PrintApplyDamageSpecKeyInfo(const FApplyDamageSpecKey& InSpecKey) const
{
	const FString actionIndexText = (InSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(InSpecKey.ActionIndex);

	FLog::Log(TEXT("---- ApplyDamageSpecKey Info ----"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(InSpecKey.WeaponType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(InSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReactionComponent::PrintReactionDataKeyInfo(const FReactionDataKey& InDataKey) const
{
	const FApplyDamageSpecKey& applyDamageSpecKey = InDataKey.ApplyDamageSpecKey;
	const FString actionIndexText = (applyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(applyDamageSpecKey.ActionIndex);

	FLog::Log(TEXT("----- ReactionDataKey Info ------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(applyDamageSpecKey.WeaponType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(applyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(InDataKey.ReactionType)));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReactionComponent::PrintReactionDataInfo(const FReactionData& InData) const
{
	const FApplyDamageSpecKey& applyDamageSpecKey = InData.ReactionDataKey.ApplyDamageSpecKey;
	const FString actionIndexText = (applyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(applyDamageSpecKey.ActionIndex);

	FLog::Log(TEXT("------ ReactionData Info --------"));
	// ApplyDamageSpec Key
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(applyDamageSpecKey.WeaponType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(applyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));

	// ReactionType Key
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(InData.ReactionDataKey.ReactionType)));

	// RactionExecutor Key
	UClass* executorClass = InData.ReactionExecutorKey.Get();
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ExecutorKey"), *GetNameSafe(executorClass)));

	// Raction Montage Data
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Montage"), *GetNameSafe(InData.Montage)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("PlayRate"), InData.PlayRate));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("bCanMove"), InData.bCanMove ? 1 : 0));
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
