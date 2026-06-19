#include "Reaction/CReaction_Parry.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

FExecutionDecisionResult UCReaction_Parry::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!IsIncomingReactionType(InQuery, EReactionType::Parry))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (InQuery.Snapshot.IsDead())
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!CanResolveExclusiveRelationship(InQuery))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = EExecutionRelationship::Exclusive;
	return result;
}

bool UCReaction_Parry::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!IsIncomingReactionType(InQuery, EReactionType::Parry)) return false;

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

void UCReaction_Parry::ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
{
	OutDecision = FObservableOverlayExecutionDecision();

	const bool bIsParryReaction = IsIncomingReactionType(InQuery.DecisionQuery, EReactionType::Parry);
	if (!bIsParryReaction)
	{
		// Parry only.
		OutDecision.Decision = EExecutionDecision::Reject;
		return;
	}

	// Guard cleanup is handled by the interrupted Guard action.
	OutDecision.Decision = EExecutionDecision::Accept;
}
