#include "Core/Debug/FActionReactionDebug.h"
#include "Core/Debug/FLog.h"

#include "Action/CAction.h"
#include "Reaction/CReaction.h"

#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarActionRequestAudit(
		TEXT("Portfolio.Debug.ActionRequestAudit"),
		0,
		TEXT("Print action request orchestration diagnostic hook logs. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarReactionRequestAudit(
		TEXT("Portfolio.Debug.ReactionRequestAudit"),
		0,
		TEXT("Print reaction request orchestration diagnostic hook logs. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarActionReactionDump(
		TEXT("Portfolio.Debug.ActionReactionDump"),
		0,
		TEXT("Print action/reaction orchestration debug dumps. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	FString FormatActionDataKey(const FActionDataKey& InKey)
	{
		return FString::Printf(
			TEXT("ActionType=%s ActionIndex=%d"),
			*UEnum::GetValueAsString(InKey.ActionType),
			InKey.ActionIndex);
	}

	FString FormatDamageSpecKey(const FDamageSpecKey& InKey)
	{
		return FString::Printf(
			TEXT("WeaponType=%s ActionType=%s ActionIndex=%d"),
			*UEnum::GetValueAsString(InKey.WeaponType),
			*UEnum::GetValueAsString(InKey.ActionType),
			InKey.ActionIndex);
	}

	FString FormatReactionDataKey(const FReactionDataKey& InKey)
	{
		return FString::Printf(
			TEXT("ReactionType=%s | %s"),
			*UEnum::GetValueAsString(InKey.ReactionType),
			*FormatDamageSpecKey(InKey.DamageSpecKey));
	}

	FString FormatParticipant(const FExecutionParticipant& InParticipant)
	{
		if (InParticipant.IsActionParticipant())
		{
			const FActionExecutionContext& context = InParticipant.GetActionContext();

			return FString::Printf(
				TEXT("Domain=%s | %s | Executor=%s | Priority=%d"),
				*UEnum::GetValueAsString(InParticipant.ParticipantDomain),
				*FormatActionDataKey(context.ActionDataKey),
				*GetNameSafe(context.ActionExecutor),
				context.ActionData.Priority);
		}

		if (InParticipant.IsReactionParticipant())
		{
			const FReactionExecutionContext& context = InParticipant.GetReactionContext();

			return FString::Printf(
				TEXT("Domain=%s | %s | Executor=%s | Priority=%d"),
				*UEnum::GetValueAsString(InParticipant.ParticipantDomain),
				*FormatReactionDataKey(context.ReactionDataKey),
				*GetNameSafe(context.ReactionExecutor),
				context.ReactionData.Priority);
		}

		return TEXT("Domain=None");
	}

	FString FormatSnapshot(const FExecutionSnapshot& InSnapshot)
	{
		return FString::Printf(
			TEXT("ExecutionState=%s | Dead=%s | GuardWants=%s | GuardPose=%s | CanGuard=%s | CanParry=%s | CanStartGuard=%s"),
			*UEnum::GetValueAsString(InSnapshot.ExecutionState),
			InSnapshot.bIsDead ? TEXT("true") : TEXT("false"),
			InSnapshot.ObservableOverlay.Guard.bWantsGuarding ? TEXT("true") : TEXT("false"),
			InSnapshot.ObservableOverlay.Guard.bIsGuardingPose ? TEXT("true") : TEXT("false"),
			InSnapshot.ObservableOverlay.Guard.bCanGuard ? TEXT("true") : TEXT("false"),
			InSnapshot.ObservableOverlay.Guard.bCanParry ? TEXT("true") : TEXT("false"),
			InSnapshot.ObservableOverlay.Guard.bCanStartGuard ? TEXT("true") : TEXT("false"));
	}

	FString FormatOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings)
	{
		if (InHandlings.IsEmpty()) return TEXT("None");

		TArray<FString> handlingNames;
		handlingNames.Reserve(InHandlings.Num());

		for (const EObservableOverlayHandling handling : InHandlings)
		{
			handlingNames.Add(UEnum::GetValueAsString(handling));
		}

		return FString::Join(handlingNames, TEXT(","));
	}
}

// Gate

bool FActionReactionDebug::ShouldAuditActionRequest()
{
#if !UE_BUILD_SHIPPING
	return CVarActionRequestAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FActionReactionDebug::ShouldAuditReactionRequest()
{
#if !UE_BUILD_SHIPPING
	return CVarReactionRequestAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FActionReactionDebug::ShouldPrintActionReactionDebug()
{
#if !UE_BUILD_SHIPPING
	return CVarActionReactionDump.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Shared Diagnostic Hook

void FActionReactionDebug::RecordInvalidActiveParticipantsForAudit(const AActor* InOwnerActor, const TCHAR* InSystem)
{
	if (!ShouldAuditActionRequest() && !ShouldAuditReactionRequest()) return;

	FLog::Log(FString::Printf(
		TEXT("[ActionReaction|%s|InvalidActiveParticipants] Owner=%s | Reason=ActionAndReactionBothActive"),
		InSystem ? InSystem : TEXT("Orchestrator"),
		*GetNameSafe(InOwnerActor)));
}

// Action Diagnostic Hook

void FActionReactionDebug::RecordActionExecutionResultForAudit(const AActor* InOwnerActor, const FActionExecutionResult& InResult, const TCHAR* InEvent)
{
	if (!ShouldAuditActionRequest()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Orchestrator|%s] Owner=%s | Decision=%s | Relationship=%s | ApplyMode=%s | RejectReason=%s | RequiresIntervention=%s | Overlay=%s | %s"),
		InEvent ? InEvent : TEXT("ExecutionResult"),
		*GetNameSafe(InOwnerActor),
		*UEnum::GetValueAsString(InResult.Decision),
		*UEnum::GetValueAsString(InResult.Relationship),
		*UEnum::GetValueAsString(InResult.ApplyMode),
		*UEnum::GetValueAsString(InResult.RejectReason),
		InResult.RequiresIntervention() ? TEXT("true") : TEXT("false"),
		*FormatOverlayHandlings(InResult.OverlayHandlings),
		*FormatActionDataKey(InResult.ResolvedContext.ActionDataKey)));
}

void FActionReactionDebug::RecordActionRequestResultForAudit(const AActor* InOwnerActor, const FActionRequestResult& InResult, const TCHAR* InEvent)
{
	if (!ShouldAuditActionRequest()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Orchestrator|%s] Owner=%s | Result=%s | RejectReason=%s"),
		InEvent ? InEvent : TEXT("RequestResult"),
		*GetNameSafe(InOwnerActor),
		*UEnum::GetValueAsString(InResult.ResultType),
		*UEnum::GetValueAsString(InResult.RejectReason)));
}

// Action Debug Dump

void FActionReactionDebug::PrintActionExecutionDebug(const AActor* InOwnerActor, const FExecutionDecisionQuery& InQuery, const FActionExecutionResult& InResult)
{
	if (!ShouldPrintActionReactionDebug()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Orchestrator|ExecutionDump] Owner=%s | %s | Incoming={%s} | Active={%s} | Decision=%s | Relationship=%s | ApplyMode=%s | RejectReason=%s | Overlay=%s"),
		*GetNameSafe(InOwnerActor),
		*FormatSnapshot(InQuery.Snapshot),
		*FormatParticipant(InQuery.IncomingPart),
		*FormatParticipant(InQuery.ActivePart),
		*UEnum::GetValueAsString(InResult.Decision),
		*UEnum::GetValueAsString(InResult.Relationship),
		*UEnum::GetValueAsString(InResult.ApplyMode),
		*UEnum::GetValueAsString(InResult.RejectReason),
		*FormatOverlayHandlings(InResult.OverlayHandlings)));
}

// Reaction Diagnostic Hook

void FActionReactionDebug::RecordReactionExecutionResultForAudit(const AActor* InOwnerActor, const FReactionExecutionResult& InResult, const TCHAR* InEvent)
{
	if (!ShouldAuditReactionRequest()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Orchestrator|%s] Owner=%s | Decision=%s | Relationship=%s | ApplyMode=%s | RejectReason=%s | RequiresIntervention=%s | Overlay=%s | %s"),
		InEvent ? InEvent : TEXT("ExecutionResult"),
		*GetNameSafe(InOwnerActor),
		*UEnum::GetValueAsString(InResult.Decision),
		*UEnum::GetValueAsString(InResult.Relationship),
		*UEnum::GetValueAsString(InResult.ApplyMode),
		*UEnum::GetValueAsString(InResult.RejectReason),
		InResult.RequiresIntervention() ? TEXT("true") : TEXT("false"),
		*FormatOverlayHandlings(InResult.OverlayHandlings),
		*FormatReactionDataKey(InResult.ResolvedContext.ReactionDataKey)));
}

void FActionReactionDebug::RecordReactionRequestResultForAudit(const AActor* InOwnerActor, const FReactionRequestResult& InResult, const TCHAR* InEvent)
{
	if (!ShouldAuditReactionRequest()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Orchestrator|%s] Owner=%s | Result=%s | RejectReason=%s"),
		InEvent ? InEvent : TEXT("RequestResult"),
		*GetNameSafe(InOwnerActor),
		*UEnum::GetValueAsString(InResult.ResultType),
		*UEnum::GetValueAsString(InResult.RejectReason)));
}

// Reaction Debug Dump

void FActionReactionDebug::PrintReactionExecutionDebug(const AActor* InOwnerActor, const FExecutionDecisionQuery& InQuery, const FReactionExecutionResult& InResult)
{
	if (!ShouldPrintActionReactionDebug()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Orchestrator|ExecutionDump] Owner=%s | %s | Incoming={%s} | Active={%s} | Decision=%s | Relationship=%s | ApplyMode=%s | RejectReason=%s | Overlay=%s"),
		*GetNameSafe(InOwnerActor),
		*FormatSnapshot(InQuery.Snapshot),
		*FormatParticipant(InQuery.IncomingPart),
		*FormatParticipant(InQuery.ActivePart),
		*UEnum::GetValueAsString(InResult.Decision),
		*UEnum::GetValueAsString(InResult.Relationship),
		*UEnum::GetValueAsString(InResult.ApplyMode),
		*UEnum::GetValueAsString(InResult.RejectReason),
		*FormatOverlayHandlings(InResult.OverlayHandlings)));
}
