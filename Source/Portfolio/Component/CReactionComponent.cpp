#include "Component/CReactionComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Reaction/CReaction.h"

#include "Type/CWeaponStructure.h"

UCReactionComponent::UCReactionComponent()
{
}

void UCReactionComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	MovementComp_Injected = InReferences.MovementComponent;
	StateComp_Injected = InReferences.StateComponent;
	HealthComp_Injected = InReferences.HealthComponent;
	ObservableOverlayComp_Injected = InReferences.ObservableOverlayComponent;
	ActionComp_Injected = InReferences.ActionComponent;
	ReactionFeedbackComp_Injected = InReferences.ReactionFeedbackComponent;

	ValidateRequiredComponentReferences();
}

bool UCReactionComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ MovementComp_Injected, TEXT("UCMovementComponent") },
		{ StateComp_Injected, TEXT("UCStateComponent") },
		{ HealthComp_Injected, TEXT("UCHealthComponent") },
		{ ObservableOverlayComp_Injected, TEXT("UCObservableOverlayComponent") },
		{ ActionComp_Injected, TEXT("UCActionComponent") },
		{ ReactionFeedbackComp_Injected, TEXT("UCReactionFeedbackComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

// Lifecycle

void UCReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeReactionRuntime();
}

void UCReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeReactionRuntime();

	Super::EndPlay(EndPlayReason);
}

// Runtime Lifecycle

void UCReactionComponent::InitializeReactionRuntime()
{
	BuildReactionRuntimeMaps();
	SetInitialActiveReactionRuntimeState();
}

void UCReactionComponent::UninitializeReactionRuntime()
{
	ResetActiveReactionRuntimeState();
	ClearReactionRuntimeMaps();
}

// Runtime Map

void UCReactionComponent::BuildReactionRuntimeMaps()
{
	BuildReactionDataMap(true);
	BuildReactionExecutorMap(true);
}

void UCReactionComponent::ClearReactionRuntimeMaps()
{
	ReactionExecutorMap.Reset();
	ReactionDataMap.Reset();
}

// Active Runtime State

void UCReactionComponent::SetInitialActiveReactionRuntimeState()
{
	ActiveReactionType = EReactionType::Idle;
}

void UCReactionComponent::ResetActiveReactionRuntimeState()
{
	ActiveReactionType = EReactionType::None;
	ActiveReactionData = FReactionData();
	ActiveReactionExecutor = nullptr;
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

	TArray<FDamageSpecKey> candidateKeys; // OutParameter
	EReactionType reactionType = InDataKey.ReactionType;
	
	// Candidate SpecKey
	BuildCandidateSpecKeys(InDataKey.DamageSpecKey, candidateKeys);

	for (const FDamageSpecKey& candidateKey : candidateKeys)
	{
		FReactionDataKey reactionDataKey;

		// Rebuild CandidateSpecKey + Type
		reactionDataKey.DamageSpecKey = candidateKey;
		reactionDataKey.ReactionType = reactionType;

		// Find ReactionData
		const FReactionData* foundPtr = ReactionDataMap.Find(reactionDataKey);
		if (!foundPtr) continue;

		const FReactionData& found = *foundPtr;
		if (!found.IsValidMinimal()) continue;

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

	const FDamageSpecKey& damageSpecKey = InData.ReactionDataKey.DamageSpecKey;
	FLog::Log(FString::Printf(
		TEXT("[ReactionComponent] Failed to resolve ReactionExecutor. ReactionType=%s | WeaponType=%s | ActionType=%s | ActionIndex=%d | ReactionExecutorKey=%s | Owner=%s"),
		*UEnum::GetValueAsString(InData.ReactionDataKey.ReactionType),
		*UEnum::GetValueAsString(damageSpecKey.WeaponType),
		*UEnum::GetValueAsString(damageSpecKey.ActionType),
		damageSpecKey.ActionIndex,
		*GetNameSafe(InData.ReactionExecutorKey.Get()),
		*GetNameSafe(OwnerCharacter_Injected)));
	return nullptr;
}

// Execution Entry

bool UCReactionComponent::ApplyReactionDecision(const FReactionExecutionResult& InResult)
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!InResult.IsAcceptedDecision()) return false;

	switch (InResult.ApplyMode)
	{
	case EExecutionApplyMode::Start:
	{
		if (!ApplyOverlayHandlings(InResult.OverlayHandlings))
		{
			return false;
		}

		return StartReaction(InResult.ResolvedContext);
	}

	case EExecutionApplyMode::Reserve:
	{
		// [NOTE] Reaction does not support reserved execution.
		return false;
	}

	case EExecutionApplyMode::Intervene:
	{
		// [NOTE] Try Apply Intervention
		if (!ApplyExecutionInterventionDirective(InResult.InterventionDirective))
		{
			return false;
		}
		if (!ApplyOverlayHandlings(InResult.OverlayHandlings))
		{
			return false;
		}

		return StartReaction(InResult.ResolvedContext);
	}
	
	default:
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
	if (!IsValid(ActionComp_Injected)) return;

	ActionComp_Injected->ConsumeDeferredAction(InConsumeKey);
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
	if (!IsValid(OwnerCharacter_Injected)) return;

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
				const FDamageSpecKey& damageSpecKey = reactionDataKey.DamageSpecKey;
				FLog::Log(FString::Printf(
					TEXT("[ReactionComponent] Duplicate ReactionData key overwritten. ReactionType=%s | WeaponType=%s | ActionType=%s | ActionIndex=%d | Owner=%s"),
					*UEnum::GetValueAsString(reactionDataKey.ReactionType),
					*UEnum::GetValueAsString(damageSpecKey.WeaponType),
					*UEnum::GetValueAsString(damageSpecKey.ActionType),
					damageSpecKey.ActionIndex,
					*GetNameSafe(OwnerCharacter_Injected)));
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
	if (!IsValid(OwnerCharacter_Injected)) return;

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
			const FDamageSpecKey& damageSpecKey = reactionData.ReactionDataKey.DamageSpecKey;
			FLog::Log(FString::Printf(
				TEXT("[ReactionComponent] Failed to add ReactionExecutor during map build. ReactionType=%s | WeaponType=%s | ActionType=%s | ActionIndex=%d | ReactionExecutorKey=%s | Owner=%s"),
				*UEnum::GetValueAsString(reactionData.ReactionDataKey.ReactionType),
				*UEnum::GetValueAsString(damageSpecKey.WeaponType),
				*UEnum::GetValueAsString(damageSpecKey.ActionType),
				damageSpecKey.ActionIndex,
				*GetNameSafe(reactionData.ReactionExecutorKey.Get()),
				*GetNameSafe(OwnerCharacter_Injected)));
			continue;
		}
	}
}

FCharacterComponentReferences UCReactionComponent::BuildReactionExecutorReferences()
{
	FCharacterComponentReferences references;

	references.OwnerCharacter = OwnerCharacter_Injected;
	references.ReactionComponent = this;
	references.ReactionFeedbackComponent = ReactionFeedbackComp_Injected;

	return references;
}

UCReaction* UCReactionComponent::AddReactionExecutor(const TSubclassOf<class UCReaction> InSubClass)
{
	UClass* executorKey = InSubClass.Get();
	if (!IsValid(executorKey)) return nullptr;

	UCReaction* add = NewObject<UCReaction>(this, InSubClass);
	if (!IsValid(add)) return nullptr;

	const FCharacterComponentReferences references = BuildReactionExecutorReferences();
	add->InitializeReferences(references);
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

void UCReactionComponent::BuildCandidateSpecKeys(const FDamageSpecKey& InSpecKey, TArray<FDamageSpecKey>& OutSpecKeys) const
{
	OutSpecKeys.Reset();

	// 1) Exact: Weapon + Action + Index
	OutSpecKeys.Add(InSpecKey);

	// 2) Any Index: Weapon + Action + AnyIndex
	{
		FDamageSpecKey candidateKey = InSpecKey;
		candidateKey.ActionIndex = INDEX_NONE;
		OutSpecKeys.Add(candidateKey);
	}

	// 3) Any Action: Weapon + AnyAction + AnyIndex
	{
		FDamageSpecKey candidateKey = InSpecKey;
		candidateKey.ActionType = EActionType::All;
		candidateKey.ActionIndex = INDEX_NONE;
		OutSpecKeys.Add(candidateKey);
	}

	// 4) Any Weapon: AnyWeapon + AnyAction + AnyIndex
	{
		FDamageSpecKey candidateKey = InSpecKey;
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
		return IsValid(ActionComp_Injected) && ActionComp_Injected->RequestInterruptActiveAction(InDirective);

	case EExecutionDomain::Reaction:
		return InterruptActiveReaction(InDirective);

	default:
		return false;
	}
}

bool UCReactionComponent::ApplyOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings)
{
	if (InHandlings.IsEmpty()) return true;

	return IsValid(ObservableOverlayComp_Injected) && ObservableOverlayComp_Injected->ApplyOverlayHandlings(InHandlings);
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
		OnReactionTypeChanged.Broadcast(OwnerCharacter_Injected, prevReactionType, ActiveReactionType);
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
		OnReactionTypeChanged.Broadcast(OwnerCharacter_Injected, prevReactionType, ActiveReactionType);
	}
}

// State Transition

void UCReactionComponent::EnterReactionState(const FReactionData& InData)
{
	if (IsValid(MovementComp_Injected) && !InData.bCanMove)
	{
		MovementComp_Injected->SetStop();
	}

	if (IsValid(StateComp_Injected))
	{
		StateComp_Injected->SetReactionState();
	}
}

void UCReactionComponent::ExitReactionState(const FReactionData& InData)
{
	const bool bAlive = IsValid(HealthComp_Injected) && HealthComp_Injected->IsAlive();
	const bool bDeadExecution = IsValid(StateComp_Injected) && StateComp_Injected->GetCurrentExecutionState() == EExecutionState::Dead;

	if (!bAlive || bDeadExecution) return;

	if (IsValid(MovementComp_Injected) && !InData.bCanMove)
	{
		MovementComp_Injected->SetMove();
	}

	if (IsValid(StateComp_Injected))
	{
		StateComp_Injected->SetIdleState();
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

void UCReactionComponent::PrintDamageSpecKeyInfo(const FDamageSpecKey& InSpecKey) const
{
	const FString actionIndexText = (InSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(InSpecKey.ActionIndex);

	FLog::Log(TEXT("---- DamageSpecKey Info ----"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(InSpecKey.WeaponType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(InSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReactionComponent::PrintReactionDataKeyInfo(const FReactionDataKey& InDataKey) const
{
	const FDamageSpecKey& damageSpecKey = InDataKey.DamageSpecKey;
	const FString actionIndexText = (damageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(damageSpecKey.ActionIndex);

	FLog::Log(TEXT("----- ReactionDataKey Info ------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(damageSpecKey.WeaponType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(damageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(InDataKey.ReactionType)));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReactionComponent::PrintReactionDataInfo(const FReactionData& InData) const
{
	const FDamageSpecKey& damageSpecKey = InData.ReactionDataKey.DamageSpecKey;
	const FString actionIndexText = (damageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : FString::FromInt(damageSpecKey.ActionIndex);

	FLog::Log(TEXT("------ ReactionData Info --------"));
	// DamageSpec Key
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(damageSpecKey.WeaponType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(damageSpecKey.ActionType)));
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
