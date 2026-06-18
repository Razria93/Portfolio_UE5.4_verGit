#include "Component/CReactionOrchestratorComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CObservableOverlayComponent.h"

#include "Action/CAction.h"
#include "Reaction/CReaction.h"

UCReactionOrchestratorComponent::UCReactionOrchestratorComponent()
{
}

// Lifecycle

void UCReactionOrchestratorComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	StateComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCStateComponent>();
	HealthComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCHealthComponent>();
	ActionComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCActionComponent>();
	ReactionComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCReactionComponent>();
	ObservableOverlayComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCObservableOverlayComponent>();
}

// Request Entry

FReactionRequestResult UCReactionOrchestratorComponent::RequestDamageReaction(const FDamageReactionRequest& InIncomingRequest)
{
	EReactionRequestRejectReason rejectReason = EReactionRequestRejectReason::None;

	if (!IsValid(ReactionComp_Cached))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, EReactionRequestRejectReason::InvalidComponent);

	if (!CanAcceptReactionRequest(rejectReason))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, rejectReason);

	FReactionCandidate candidate;

	if (!ResolveDamageReactionCandidate(InIncomingRequest, candidate, rejectReason))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, rejectReason);

	return ProcessReactionCandidate(candidate);
}

// Request Validation

bool UCReactionOrchestratorComponent::CanAcceptReactionRequest(EReactionRequestRejectReason& OutRejectReason) const
{
	OutRejectReason = EReactionRequestRejectReason::None;

	if (!IsValid(OwnerCharacter_Cached))
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidOwner;
		return false;
	}

	if (!IsValid(ReactionComp_Cached) || !IsValid(StateComp_Cached) || !IsValid(HealthComp_Cached))
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidComponent;
		return false;
	}

	// [NOTE]
	// Do not reject dead state here.
	// DeadReaction may be requested after health and dead state have already been committed.

	return true;
}

// Candidate Resolve

bool UCReactionOrchestratorComponent::ResolveDamageReactionCandidate(const FDamageReactionRequest& InIncomingRequest, FReactionCandidate& OutIncomingCandidate, EReactionRequestRejectReason& OutRejectReason) const
{
	OutIncomingCandidate = FReactionCandidate();
	OutRejectReason = EReactionRequestRejectReason::None;

	if (InIncomingRequest.IntentSource != EReactionIntentSource::TakeDamage)
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidRequest;
		return false;
	}

	if (!InIncomingRequest.TakeDamagePacket.Result.bAccepted)
	{
		OutRejectReason = EReactionRequestRejectReason::InvalidDamageResult;
		return false;
	}

	const EReactionType reactionType = ResolveDamageReactionType(InIncomingRequest);

	if (reactionType == EReactionType::None || reactionType == EReactionType::Max)
	{
		OutRejectReason = EReactionRequestRejectReason::ReactionCandidateNotFound;
		return false;
	}

	OutIncomingCandidate.ReactionDataKey.ApplyDamageSpecKey = InIncomingRequest.TakeDamagePacket.Result.ApplyDamageSpecKey;
	OutIncomingCandidate.ReactionDataKey.ReactionType = reactionType;
	return true;
}

EReactionType UCReactionOrchestratorComponent::ResolveDamageReactionType(const FDamageReactionRequest& InIncomingRequest) const
{
	const FTakeDamageResult& damageResult = InIncomingRequest.TakeDamagePacket.Result;

	if (!damageResult.bAccepted) return EReactionType::None;

	if (damageResult.DeadState_Before == EDeadState::Alive && damageResult.DeadState_After != EDeadState::Alive)
	{
		return EReactionType::Dead;
	}

	if (damageResult.DefenseOutcome == EDamageDefenseOutcome::Parry)
	{
		return EReactionType::Parry;
	}

	if (damageResult.DefenseOutcome == EDamageDefenseOutcome::Guard)
	{
		return EReactionType::BlockHit;
	}

	if (damageResult.CommittedDamage > KINDA_SMALL_NUMBER && damageResult.DeadState_After == EDeadState::Alive)
	{
		return EReactionType::Hit;
	}

	return EReactionType::None;
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

	return true;
}

bool UCReactionOrchestratorComponent::ResolveReactionData(const FReactionDataKey& InIncomingDataKey, FReactionData& OutIncomingData) const
{
	OutIncomingData = FReactionData();

	if (!IsValid(ReactionComp_Cached)) return false;
	if (!InIncomingDataKey.IsValidMinimal()) return false;

	return ReactionComp_Cached->ResolveReactionData(InIncomingDataKey, OutIncomingData);
}

UCReaction* UCReactionOrchestratorComponent::ResolveReactionExecutor(const FReactionData& InIncomingData) const
{
	if (!IsValid(ReactionComp_Cached)) return nullptr;
	if (!InIncomingData.IsValidMinimal()) return nullptr;

	// Resolve Executor
	return ReactionComp_Cached->ResolveReactionExecutor(InIncomingData);
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

	snapshot.ExecutionState = IsValid(StateComp_Cached) ? StateComp_Cached->GetCurrentExecutionState() : EExecutionState::Dead;
	snapshot.bIsDead = !IsValid(HealthComp_Cached) || !HealthComp_Cached->IsAlive();

	if (IsValid(ObservableOverlayComp_Cached))
	{
		ObservableOverlayComp_Cached->WriteObservableOverlaySnapshot(snapshot.ObservableOverlay);
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

	const bool bHasActiveAction = IsValid(ActionComp_Cached) && ActionComp_Cached->IsActive();
	const bool bHasActiveReaction = IsValid(ReactionComp_Cached) && ReactionComp_Cached->IsActive();

	if (bHasActiveAction && bHasActiveReaction)
	{
		FLog::Log(TEXT("[ReactionOrchestrator] Invalid execution state (action and reaction are both active)."));
		return participant;
	}

	// 01. Active Reaction Case
	if (bHasActiveReaction)
	{
		FReactionData activeData;

		if (ReactionComp_Cached->GetActiveReactionData(activeData))
		{
			FReactionExecutionContext context;

			context.ReactionDataKey = activeData.ReactionDataKey;
			context.ReactionData = activeData;
			context.ReactionExecutor = ReactionComp_Cached->GetActiveReactionExecutor();

			if (context.IsValidMinimal())
			{
				participant.bIsValid = true;
				participant.ParticipantDomain = EExecutionDomain::Reaction;
				participant.ReactionContext = context;

				return participant;
			}
		}
	}

	// 02. Active Action Case
	if (bHasActiveAction)
	{
		FActionData activeData;

		if (ActionComp_Cached->GetActiveActionData(activeData))
		{
			FActionExecutionContext context;

			context.ActionDataKey = activeData.ActionDataKey;
			context.ActionData = activeData;
			context.ActionExecutor = ActionComp_Cached->GetActiveActionExecutor();

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

	// [NOTE] Early return ignore and reject decision
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
		// [NOTE] Reaction does not support sequential execution.
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

	// [NOTE] Start immediately when there is no active execution.
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
		// [NOTE]
		// Force intervention.
		// Do not ask incoming WantIntervention or active AllowIntervention.
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

	FLog::Log(FString::Printf(
		TEXT("[ResolveInterventionDirective] Owner = %s | bIncomingWants = %s | bActiveAllows = %s"),
		*GetNameSafe(OwnerCharacter_Cached),
		bIncomingWants ? TEXT("true") : TEXT("false"),
		bActiveAllows ? TEXT("true") : TEXT("false")));

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
	// [NOTE] Request ignore result
	if (InResult.Decision == EExecutionDecision::Ignore)
		return BuildReactionRequestResult(EReactionRequestResultType::Ignored, EReactionRequestRejectReason::None);

	// [NOTE] Request rejected result
	if (!InResult.IsAcceptedDecision())
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, InResult.RejectReason);

	if (!IsValid(ReactionComp_Cached))
		return BuildReactionRequestResult(EReactionRequestResultType::Rejected, EReactionRequestRejectReason::InvalidComponent);

	if (!ReactionComp_Cached->ApplyReactionDecision(InResult))
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
		PrintReactionRequestResult(result);
	}
	else
	{
		result.RejectReason = EReactionRequestRejectReason::None;
	}

	return result;
}

// Debug

void UCReactionOrchestratorComponent::PrintReactionRequestResult(const FReactionRequestResult& InResult) const
{
	FLog::Log(FString::Printf(
		TEXT("[ReactionRequestResult] Owner = %s | ResultType = %s | RejectReason = %s"),
		*GetNameSafe(OwnerCharacter_Cached),
		*UEnum::GetValueAsString(InResult.ResultType),
		*UEnum::GetValueAsString(InResult.RejectReason)
	));
}
