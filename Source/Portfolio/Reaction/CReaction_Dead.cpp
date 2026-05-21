#include "Reaction/CReaction_Dead.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

FExecutionDecisionResult UCReaction_Dead::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!IsIncomingReactionType(InQuery, EReactionType::Dead))
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

bool UCReaction_Dead::MatchesWantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (Super::MatchesWantIntervention(InQuery)) return true;
	if (!IsIncomingReactionType(InQuery, EReactionType::Dead)) return false;

	return InQuery.StopReason == EExecutionStopReason::Interrupted;
}

bool UCReaction_Dead::MatchesAllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	// [NOTE] Dead reaction is terminal and cannot be interrupted or cancelled.
	return false;
}
