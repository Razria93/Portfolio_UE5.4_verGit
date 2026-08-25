#include "Component/CReactionOrchestratorComponent.h"

#include "ProjectGlobal.h"

#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Action/CAction.h"
#include "Reaction/CReaction.h"
#include "Core/Debug/FExecutionOrchestratorDebug.h"

#include "GameFramework/Character.h"

UCReactionOrchestratorComponent::UCReactionOrchestratorComponent()
{
}

void UCReactionOrchestratorComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	StateComp_Injected = InReferences.StateComponent;
	HealthComp_Injected = InReferences.HealthComponent;
	ObservableOverlayComp_Injected = InReferences.ObservableOverlayComponent;
	ActionComp_Injected = InReferences.ActionComponent;
	ReactionComp_Injected = InReferences.ReactionComponent;

	ValidateRequiredComponentReferences();
}

bool UCReactionOrchestratorComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ StateComp_Injected, TEXT("UCStateComponent") },
		{ HealthComp_Injected, TEXT("UCHealthComponent") },
		{ ActionComp_Injected, TEXT("UCActionComponent") },
		{ ReactionComp_Injected, TEXT("UCReactionComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

// Request Entry

FReactionRequestResult UCReactionOrchestratorComponent::RequestDamageReaction(const FDamageReactionRequest& InIncomingRequest)
{
	EReactionRequestRejectReason rejectReason = EReactionRequestRejectReason::None;

	if (!IsValid(ReactionComp_Injected))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, EReactionRequestRejectReason::InvalidComponent);

	if (!CanAcceptReactionRequest(rejectReason))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, rejectReason);

	FReactionCandidate candidate;

	if (!ResolveDamageReactionCandidate(InIncomingRequest, candidate, rejectReason))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, rejectReason);

	return ProcessReactionCandidate(candidate);
}

FReactionRequestResult UCReactionOrchestratorComponent::RequestBalanceLifecycleReaction(const FBalanceLifecycleReactionRequest& InIncomingRequest)
{
	EReactionRequestRejectReason rejectReason = EReactionRequestRejectReason::None;

	if (!IsValid(ReactionComp_Injected))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, EReactionRequestRejectReason::InvalidComponent);

	if (!CanAcceptReactionRequest(rejectReason))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, rejectReason);

	FReactionCandidate candidate;

	if (!ResolveBalanceLifecycleReactionCandidate(InIncomingRequest, candidate, rejectReason))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, rejectReason);

	return ProcessReactionCandidate(candidate);
}

// Request Validation

bool UCReactionOrchestratorComponent::CanAcceptReactionRequest(EReactionRequestRejectReason& OutRejectReason) const
{
	OutRejectReason = EReactionRequestRejectReason::None;

	if (!IsValid(OwnerCharacter_Injected))
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidOwner;
		return false;
	}

	if (!IsValid(ReactionComp_Injected) || !IsValid(StateComp_Injected) || !IsValid(HealthComp_Injected))
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidComponent;
		return false;
	}

	// DeadReaction may be requested after health and dead state have already been committed.

	return true;
}

// Candidate Resolve

bool UCReactionOrchestratorComponent::ResolveDamageReactionCandidate(const FDamageReactionRequest& InIncomingRequest, FReactionCandidate& OutIncomingCandidate, EReactionRequestRejectReason& OutRejectReason) const
{
	OutIncomingCandidate = FReactionCandidate();
	OutRejectReason = EReactionRequestRejectReason::None;

	if (InIncomingRequest.IntentSource != EReactionIntentSource::CombatSignalTarget)
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidRequest;
		return false;
	}

	if (!InIncomingRequest.CombatSignalTargetPacket.Result.bAccepted)
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidDamageResult;
		return false;
	}

	const EReactionType reactionType = ResolveDamageReactionType(InIncomingRequest.CombatSignalTargetPacket.Result.ReactionOutcome);

	if (reactionType == EReactionType::None || reactionType == EReactionType::Max)
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionCandidateNotFound;
		return false;
	}

	OutIncomingCandidate.ReactionDataKey.DamageSpecKey = InIncomingRequest.CombatSignalTargetPacket.Result.DamageSpecKey;
	OutIncomingCandidate.ReactionDataKey.MatchMode = EReactionDataMatchMode::DamageSpec;
	OutIncomingCandidate.ReactionDataKey.ReactionType = reactionType;
	OutIncomingCandidate.CombatSignalResultSerial = InIncomingRequest.CombatSignalTargetPacket.ResultSerial;
	return true;
}

EReactionType UCReactionOrchestratorComponent::ResolveDamageReactionType(const EDamageReactionOutcome InOutcome) const
{
	switch (InOutcome)
	{
	case EDamageReactionOutcome::Hit: return EReactionType::Hit;
	case EDamageReactionOutcome::BlockHit: return EReactionType::BlockHit;
	case EDamageReactionOutcome::Parry: return EReactionType::Parry;
	case EDamageReactionOutcome::CollapseHit: return EReactionType::CollapseHit;
	case EDamageReactionOutcome::Dead: return EReactionType::Dead;
	default: return EReactionType::None;
	}
}

bool UCReactionOrchestratorComponent::ResolveBalanceLifecycleReactionCandidate(const FBalanceLifecycleReactionRequest& InIncomingRequest, FReactionCandidate& OutIncomingCandidate, EReactionRequestRejectReason& OutRejectReason) const
{
	OutIncomingCandidate = FReactionCandidate();
	OutRejectReason = EReactionRequestRejectReason::None;

	if (InIncomingRequest.IntentSource != EReactionIntentSource::BalanceLifecycle
		|| InIncomingRequest.BalanceLifecycleSerial == 0)
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidRequest;
		return false;
	}

	if (InIncomingRequest.ReactionType != EReactionType::CollapseIn
		&& InIncomingRequest.ReactionType != EReactionType::CollapseOut)
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionCandidateNotFound;
		return false;
	}

	OutIncomingCandidate.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
	OutIncomingCandidate.ReactionDataKey.ReactionType = InIncomingRequest.ReactionType;
	OutIncomingCandidate.ReactionDataKey.ReactionIndex = INDEX_NONE;
	OutIncomingCandidate.BalanceLifecycleSerial = InIncomingRequest.BalanceLifecycleSerial;
	return true;
}

// Orchestration Pipeline

FReactionRequestResult UCReactionOrchestratorComponent::ProcessReactionCandidate(const FReactionCandidate& InIncomingCandidate)
{
	EReactionRequestRejectReason rejectReason = EReactionRequestRejectReason::None;

	FReactionExecutionContext incomingContext;

	if (!ResolveReactionContext(InIncomingCandidate, incomingContext, rejectReason))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, rejectReason);

	const FExecutionDecisionQuery decisionQuery = BuildDecisionQuery(incomingContext);
	const FExecutionDecisionResult decisionResult = BuildDecisionResult(decisionQuery, rejectReason);
	FReactionExecutionResult executionResult = BuildReactionExecutionResult(incomingContext, decisionResult, rejectReason);

	ResolveExecutionApplyMode(decisionQuery, executionResult);
	ResolveObservableOverlayGate(decisionQuery, executionResult);

	FExecutionOrchestratorDebug::RecordReactionExecutionResultForAudit(OwnerCharacter_Injected, executionResult, TEXT("DecisionResolved"));
	FExecutionOrchestratorDebug::PrintReactionExecutionDebug(OwnerCharacter_Injected, decisionQuery, executionResult);

	return DispatchReactionDecision(executionResult);
}

// Execution Context Resolve

bool UCReactionOrchestratorComponent::ResolveReactionContext(const FReactionCandidate& InIncomingCandidate, FReactionExecutionContext& OutIncomingContext, EReactionRequestRejectReason& OutRejectReason) const
{
	OutIncomingContext = FReactionExecutionContext();
	OutRejectReason = EReactionRequestRejectReason::None;

	if (!InIncomingCandidate.IsValidMinimal())
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidRequest;
		return false;
	}

	FReactionDataKey incomingReactionDataKey = InIncomingCandidate.ReactionDataKey;

	FReactionData incomingReactionData;
	if (!ResolveReactionData(incomingReactionDataKey, incomingReactionData))
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionDataNotFound;
		return false;
	}

	UCReaction* incomingReactionExecutor = ResolveReactionExecutor(incomingReactionData);
	if (!IsValid(incomingReactionExecutor))
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionExecutorNotFound;
		return false;
	}

	OutIncomingContext.ReactionDataKey = incomingReactionDataKey;
	OutIncomingContext.ReactionData = incomingReactionData;
	OutIncomingContext.ReactionExecutor = incomingReactionExecutor;
	OutIncomingContext.CombatSignalResultSerial = InIncomingCandidate.CombatSignalResultSerial;
	OutIncomingContext.BalanceLifecycleSerial = InIncomingCandidate.BalanceLifecycleSerial;

	return true;
}

bool UCReactionOrchestratorComponent::ResolveReactionData(const FReactionDataKey& InIncomingDataKey, FReactionData& OutIncomingData) const
{
	OutIncomingData = FReactionData();

	if (!IsValid(ReactionComp_Injected)) return false;
	if (!InIncomingDataKey.IsValidMinimal()) return false;

	return ReactionComp_Injected->ResolveReactionData(InIncomingDataKey, OutIncomingData);
}

UCReaction* UCReactionOrchestratorComponent::ResolveReactionExecutor(const FReactionData& InIncomingData) const
{
	if (!IsValid(ReactionComp_Injected)) return nullptr;
	if (!InIncomingData.IsValidMinimal()) return nullptr;

	// Resolve the executor from the selected reaction data.
	return ReactionComp_Injected->ResolveReactionExecutor(InIncomingData);
}

// Decision Query Build

FExecutionDecisionQuery UCReactionOrchestratorComponent::BuildDecisionQuery(const FReactionExecutionContext& InIncomingContext) const
{
	FExecutionDecisionQuery query;

	query.Snapshot = BuildSnapshot();
	query.IncomingPart = BuildIncomingReactionParticipant(InIncomingContext);
	query.ActivePart = BuildActiveExecutionParticipant();

	return query;
}

FExecutionSnapshot UCReactionOrchestratorComponent::BuildSnapshot() const
{
	FExecutionSnapshot snapshot;

	snapshot.ExecutionState = IsValid(StateComp_Injected) ? StateComp_Injected->GetCurrentExecutionState() : EExecutionState::Max;
	snapshot.bIsDead = !IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive();

	if (IsValid(ObservableOverlayComp_Injected))
	{
		ObservableOverlayComp_Injected->WriteOverlaySnapshot(snapshot.ObservableOverlay);
	}

	return snapshot;
}

FExecutionParticipant UCReactionOrchestratorComponent::BuildIncomingReactionParticipant(const FReactionExecutionContext& InIncomingContext) const
{
	FExecutionParticipant participant;

	if (!InIncomingContext.IsValidMinimal()) return participant;

	participant.bIsValid = true;
	participant.ParticipantDomain = EExecutionDomain::Reaction;
	participant.ReactionContext = InIncomingContext;

	return participant;
}

FExecutionParticipant UCReactionOrchestratorComponent::BuildActiveExecutionParticipant() const
{
	FExecutionParticipant participant;

	const bool bHasActiveAction = IsValid(ActionComp_Injected) && ActionComp_Injected->IsActive();
	const bool bHasActiveReaction = IsValid(ReactionComp_Injected) && ReactionComp_Injected->IsActive();

	if (bHasActiveAction && bHasActiveReaction)
	{
		FExecutionOrchestratorDebug::RecordInvalidActiveParticipantsForAudit(OwnerCharacter_Injected, TEXT("ReactionOrchestrator"));
		return participant;
	}

	// Preferred: active reaction participant.
	if (bHasActiveReaction)
	{
		FReactionExecutionContext context;

		if (ReactionComp_Injected->GetActiveReactionContext(context))
		{
			if (context.IsValidMinimal())
			{
				participant.bIsValid = true;
				participant.ParticipantDomain = EExecutionDomain::Reaction;
				participant.ReactionContext = context;

				return participant;
			}
		}
	}

	// Fallback: active action participant.
	if (bHasActiveAction)
	{
		FActionData activeData;

		if (ActionComp_Injected->GetActiveActionData(activeData))
		{
			FActionExecutionContext context;

			context.ActionDataKey = activeData.ActionDataKey;
			context.ActionData = activeData;
			context.ActionExecutor = ActionComp_Injected->GetActiveActionExecutor();

			if (context.IsValidMinimal())
			{
				participant.bIsValid = true;
				participant.ParticipantDomain = EExecutionDomain::Action;
				participant.ActionContext = context;

				return participant;
			}
		}
	}

	return participant;
}

// Decision Build

FExecutionDecisionResult UCReactionOrchestratorComponent::BuildDecisionResult(const FExecutionDecisionQuery& InQuery, EReactionRequestRejectReason& OutRejectReason) const
{
	FExecutionDecisionResult result;
	OutRejectReason = EReactionRequestRejectReason::None;

	if (!InQuery.HasIncomingPart())
	{
		result.Decision = EExecutionDecision::Reject;
		OutRejectReason = EReactionRequestRejectReason::InvalidQuery;

		return result;
	}

	if (!InQuery.IncomingPart.IsReactionParticipant())
	{
		result.Decision = EExecutionDecision::Reject;
		OutRejectReason = EReactionRequestRejectReason::InvalidQuery;

		return result;
	}

	const FReactionExecutionContext& incomingContext = InQuery.IncomingPart.GetReactionContext();
	UCReaction* incomingExecutor = incomingContext.ReactionExecutor;

	if (!IsValid(incomingExecutor))
	{
		result.Decision = EExecutionDecision::Reject;
		OutRejectReason = EReactionRequestRejectReason::ReactionExecutorNotFound;

		return result;
	}

	result = incomingExecutor->ResolveExecutionDecision(InQuery);

	if (result.Decision == EExecutionDecision::Reject)
	{
		OutRejectReason = EReactionRequestRejectReason::RejectedByExecutor;
	}

	return result;
}

FReactionExecutionResult UCReactionOrchestratorComponent::BuildReactionExecutionResult(const FReactionExecutionContext& InContext, const FExecutionDecisionResult& InDecisionResult, EReactionRequestRejectReason InRejectReason) const
{
	FReactionExecutionResult result;

	result.Decision = InDecisionResult.Decision;
	result.Relationship = InDecisionResult.Relationship;
	result.ApplyMode = EExecutionApplyMode::None;
	result.ResolvedContext = InContext;
	result.RejectReason = InRejectReason;

	return result;
}

// Decision Refinement

void UCReactionOrchestratorComponent::ResolveExecutionApplyMode(const FExecutionDecisionQuery& InQuery, FReactionExecutionResult& InOutResult) const
{
	InOutResult.ApplyMode = EExecutionApplyMode::None;
	InOutResult.InterventionDirective = FExecutionInterventionDirective();

	if (!InOutResult.IsAcceptedDecision()) return;

	switch (InOutResult.Relationship)
	{
	case EExecutionRelationship::Independent:
	{
		if (!(InQuery.Snapshot.IsIdle() && !InQuery.HasActivePart()))
		{
			InOutResult.Decision = EExecutionDecision::Reject;
			InOutResult.RejectReason = EReactionRequestRejectReason::InvalidIndependent;
			return;
		}

		InOutResult.ApplyMode = EExecutionApplyMode::Start;
		return;
	}

	case EExecutionRelationship::Sequential:
	{
		// Reaction does not support sequential execution.
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = EReactionRequestRejectReason::InvalidSequential;
		return;

	}

	case EExecutionRelationship::Exclusive:
	{
		if (InQuery.Snapshot.IsIdle() || !InQuery.HasActivePart())
		{
			InOutResult.Decision = EExecutionDecision::Reject;
			InOutResult.RejectReason = EReactionRequestRejectReason::InvalidExclusive;
			return;
		}

		ResolveInterventionDirective(InQuery, InOutResult);

		if (!InOutResult.IsAcceptedDecision()) return;

		if (!InOutResult.InterventionDirective.IsRequested())
		{
			InOutResult.Decision = EExecutionDecision::Reject;
			InOutResult.RejectReason = EReactionRequestRejectReason::InvalidExclusive;
			return;
		}

		InOutResult.ApplyMode = EExecutionApplyMode::Intervene;
		return;
	}

	default:
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = EReactionRequestRejectReason::NoExecutableReaction;
		return;
	}
}

void UCReactionOrchestratorComponent::ResolveInterventionDirective(const FExecutionDecisionQuery& InQuery, FReactionExecutionResult& InOutResult) const
{
	InOutResult.InterventionDirective = FExecutionInterventionDirective();

	if (!InOutResult.IsAcceptedDecision()) return;

	if (!InQuery.HasActivePart()) return;

	FExecutionInterventionQuery interventionQuery;

	if (!BuildInterventionQuery(InQuery, EExecutionStopReason::Interrupted, interventionQuery))
	{
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = EReactionRequestRejectReason::InvalidQuery;
		return;
	}

	const FExecutionParticipant& incoming = interventionQuery.IncomingPart;
	const FExecutionParticipant& active = interventionQuery.ActivePart;

	const bool bIncomingDead = incoming.IsReactionParticipant() && incoming.GetReactionContext().ReactionDataKey.ReactionType == EReactionType::Dead;

	if (bIncomingDead)
	{
		// DeadReaction forces intervention without querying participant permissions.
		FExecutionInterventionDirective directive;

		if (!BuildInterventionDirective(interventionQuery, EExecutionStopSource::ReactionOrchestration, EExecutionAfterStopAction::StartIncoming, directive))
		{
			InOutResult.Decision = EExecutionDecision::Reject;
			InOutResult.RejectReason = EReactionRequestRejectReason::InterventionDispatchFailed;
			return;
		}

		InOutResult.InterventionDirective = directive;
		return;
	}

	bool bIncomingWants = false;
	bool bActiveAllows = false;

	UCReaction* incomingReaction = Cast<UCReaction>(incoming.GetExecutor());
	if (!IsValid(incomingReaction))
	{
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = EReactionRequestRejectReason::ReactionExecutorNotFound;
		return;
	}

	bIncomingWants = incomingReaction->WantIntervention(interventionQuery);

	if (UCAction* activeAction = Cast<UCAction>(active.GetExecutor()))
	{
		bActiveAllows = activeAction->AllowIntervention(interventionQuery);
	}
	else if (UCReaction* activeReaction = Cast<UCReaction>(active.GetExecutor()))
	{
		bActiveAllows = activeReaction->AllowIntervention(interventionQuery);
	}

	if (!bIncomingWants)
	{
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = EReactionRequestRejectReason::IncomingCannotIntervene;
		return;
	}

	if (!bActiveAllows)
	{
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = EReactionRequestRejectReason::ActiveCannotAcceptIntervention;
		return;
	}

	FExecutionInterventionDirective directive;

	if (!BuildInterventionDirective(interventionQuery, EExecutionStopSource::ReactionOrchestration, EExecutionAfterStopAction::StartIncoming, directive))
	{
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = EReactionRequestRejectReason::InterventionDispatchFailed;
		return;
	}

	InOutResult.InterventionDirective = directive;
}

void UCReactionOrchestratorComponent::ResolveObservableOverlayGate(const FExecutionDecisionQuery& InQuery, FReactionExecutionResult& InOutResult) const
{
	InOutResult.OverlayHandlings.Empty();

	if (!InOutResult.IsAcceptedDecision()) return;

	const bool bNeedsExecutionStart = InOutResult.ApplyMode == EExecutionApplyMode::Start || InOutResult.ApplyMode == EExecutionApplyMode::Intervene;
	if (!bNeedsExecutionStart) return;

	FObservableOverlayQuery overlayQuery;
	overlayQuery.DecisionQuery = InQuery;
	overlayQuery.ApplyMode = InOutResult.ApplyMode;

	if (InQuery.IncomingPart.IsReactionParticipant())
	{
		if (const UCReaction* incomingReaction = InQuery.IncomingPart.GetReactionContext().ReactionExecutor)
		{
			FObservableOverlayExecutionDecision overlayDecision;
			incomingReaction->ResolveObservableOverlayCondition(overlayQuery, overlayDecision);

			if (!overlayDecision.IsAccepted())
			{
				InOutResult.Decision = overlayDecision.Decision;
				InOutResult.RejectReason = EReactionRequestRejectReason::RejectedByOverlay;
				return;
			}

			for (const EObservableOverlayHandling handling : overlayDecision.Handlings)
			{
				if (handling == EObservableOverlayHandling::None) continue;

				InOutResult.OverlayHandlings.AddUnique(handling);
			}
		}
	}
}

// Intervention Build

bool UCReactionOrchestratorComponent::BuildInterventionQuery(const FExecutionDecisionQuery& InQuery, EExecutionStopReason InStopReason, FExecutionInterventionQuery& OutQuery) const
{
	OutQuery = FExecutionInterventionQuery();

	if (!InQuery.IncomingPart.IsValidMinimal()) return false;
	if (!InQuery.ActivePart.IsValidMinimal()) return false;

	if (InStopReason == EExecutionStopReason::None || InStopReason == EExecutionStopReason::Max) return false;

	OutQuery.Snapshot = InQuery.Snapshot;
	OutQuery.IncomingPart = InQuery.IncomingPart;
	OutQuery.ActivePart = InQuery.ActivePart;
	OutQuery.StopReason = InStopReason;

	return OutQuery.IsValidMinimal();
}

bool UCReactionOrchestratorComponent::BuildInterventionDirective(const FExecutionInterventionQuery& InQuery, EExecutionStopSource InStopSource, EExecutionAfterStopAction InAfterStopAction, FExecutionInterventionDirective& OutDirective) const
{
	OutDirective = FExecutionInterventionDirective();

	if (!InQuery.IsValidMinimal()) return false;

	if (InStopSource == EExecutionStopSource::None || InStopSource == EExecutionStopSource::Max) return false;
	if (InAfterStopAction == EExecutionAfterStopAction::None || InAfterStopAction == EExecutionAfterStopAction::Max) return false;

	OutDirective.bRequested = true;
	OutDirective.StopSource = InStopSource;
	OutDirective.SourceDomain = InQuery.IncomingPart.ParticipantDomain;
	OutDirective.TargetDomain = InQuery.ActivePart.ParticipantDomain;
	OutDirective.StopReason = InQuery.StopReason;
	OutDirective.AfterStopAction = InAfterStopAction;
	OutDirective.IncomingPart = InQuery.IncomingPart;
	OutDirective.ActivePart = InQuery.ActivePart;

	return OutDirective.IsValidRequest();
}

// Decision Dispatch

FReactionRequestResult UCReactionOrchestratorComponent::DispatchReactionDecision(const FReactionExecutionResult& InResult)
{
	if (InResult.Decision == EExecutionDecision::Ignore)
		return BuildReactionRequestResult(EReactionRequestResultType::Ignored, EReactionRequestRejectReason::None);

	if (!InResult.IsAcceptedDecision())
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, InResult.RejectReason);

	if (!IsValid(ReactionComp_Injected))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, EReactionRequestRejectReason::InvalidComponent);

	if (!ReactionComp_Injected->ApplyReactionDecision(InResult))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, EReactionRequestRejectReason::ReactionExecutionFailed);

	const EReactionRequestResultType resultType = ConvertDecisionToResultType(InResult);

	return BuildReactionRequestResult(resultType);
}

// Result Build

EReactionRequestResultType UCReactionOrchestratorComponent::ConvertDecisionToResultType(const FReactionExecutionResult& InResult) const
{
	if (InResult.Decision == EExecutionDecision::Reject)
		return EReactionRequestResultType::Rejected;

	if (InResult.Decision == EExecutionDecision::Ignore)
		return EReactionRequestResultType::Ignored;

	switch (InResult.ApplyMode)
	{
	case EExecutionApplyMode::Start:
		return EReactionRequestResultType::Started;

	case EExecutionApplyMode::Intervene:
		return EReactionRequestResultType::Intervened;

	case EExecutionApplyMode::Reserve:
	default:
		return EReactionRequestResultType::None;
	}
}

FReactionRequestResult UCReactionOrchestratorComponent::BuildReactionRequestResult(EReactionRequestResultType InResultType, EReactionRequestRejectReason InRejectReason) const
{
	FReactionRequestResult result;

	result.ResultType = InResultType;
	result.RejectReason = InRejectReason;

	if (InResultType == EReactionRequestResultType::Rejected)
	{
		result.RejectReason = (InRejectReason != EReactionRequestRejectReason::None) ? InRejectReason : EReactionRequestRejectReason::NoExecutableReaction;
	}
	else
	{
		result.RejectReason = EReactionRequestRejectReason::None;
	}

	FExecutionOrchestratorDebug::RecordReactionRequestResultForAudit(OwnerCharacter_Injected, result, TEXT("RequestResult"));

	return result;
}
