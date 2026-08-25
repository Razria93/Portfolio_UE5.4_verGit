#include "Reaction/CReaction_Execution.h"

#include "GameFramework/Character.h"

FExecutionDecisionResult UCReaction_Execution::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;
	if (!IsValid(OwnerCharacter_Injected)
		|| !IsIncomingReactionType(InQuery, EReactionType::Execution)
		|| !CanResolveIndependentRelationship(InQuery))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = EExecutionRelationship::Independent;
	return result;
}
