#include "Reaction/CReaction_CollapseHit.h"

#include "GameFramework/Character.h"

// Decision

FExecutionDecisionResult UCReaction_CollapseHit::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected)
		|| !IsIncomingReactionType(InQuery, EReactionType::CollapseHit)
		|| InQuery.Snapshot.IsDead())
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
