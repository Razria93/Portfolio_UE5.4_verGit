#include "Action/CAction_Execution.h"

#include "GameFramework/Character.h"

FExecutionDecisionResult UCAction_Execution::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;
	if (!IsValid(OwnerCharacter_Injected)
		|| !IsIncomingActionType(InQuery, EActionType::Execution)
		|| !CanResolveIndependentRelationship(InQuery))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = EExecutionRelationship::Independent;
	return result;
}
