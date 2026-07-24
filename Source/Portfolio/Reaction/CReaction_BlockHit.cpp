#include "Reaction/CReaction_BlockHit.h"

#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

FExecutionDecisionResult UCReaction_BlockHit::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!IsIncomingReactionType(InQuery, EReactionType::BlockHit))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (InQuery.Snapshot.IsDead())
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	EExecutionRelationship relationship = EExecutionRelationship::None;

	if (!TryResolveIndependentOrExclusiveRelationship(InQuery, relationship))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = relationship;
	return result;
}

void UCReaction_BlockHit::Complete()
{
	Super::Complete();

	RequestConsumeDeferredAction(EDeferredActionConsumeKey::AfterGuardBlockReaction);
	RequestConsumeDeferredAction(EDeferredActionConsumeKey::AfterGuardInAction);
}

bool UCReaction_BlockHit::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!IsIncomingReactionType(InQuery, EReactionType::BlockHit)) return false;

	if (Super::WantIntervention(InQuery)) return true;

	if (!InQuery.ActivePart.IsActionParticipant()) return false;

	const FActionExecutionContext& activeContext = InQuery.ActivePart.GetActionContext();
	const EGuardActionPhase activeGuardPhase = ResolveGuardActionPhase(activeContext.ActionDataKey);

	if (activeContext.ActionDataKey.ActionType == EActionType::Guard && activeGuardPhase == EGuardActionPhase::In)
	{
		return true;
	}

	return false;
}

void UCReaction_BlockHit::ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
{
	OutDecision = FObservableOverlayExecutionDecision();

	const bool bIsBlockHitReaction = IsIncomingReactionType(InQuery.DecisionQuery, EReactionType::BlockHit);
	if (!bIsBlockHitReaction)
	{
		// BlockHit only.
		OutDecision.Decision = EExecutionDecision::Reject;
		return;
	}

	const FGuardObservableOverlaySnapshot& guardSnapshot = InQuery.DecisionQuery.Snapshot.ObservableOverlay.Guard;
	if (guardSnapshot.bCanGuard)
	{
		// Guard Case: keep Guard during BlockHit.
		OutDecision.Decision = EExecutionDecision::Accept;
		return;
	}

	// Another Case: BlockHit requires an active Guard window.
	OutDecision.Decision = EExecutionDecision::Reject;
}
