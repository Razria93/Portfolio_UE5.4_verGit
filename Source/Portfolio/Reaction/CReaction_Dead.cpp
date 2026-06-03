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

bool UCReaction_Dead::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!IsIncomingReactionType(InQuery, EReactionType::Dead)) return false;

	// [Condition of WantIntervention] Just Want Interruption. No Cancel.
	bool result = InQuery.StopReason == EExecutionStopReason::Interrupted;

	return result;
}

bool UCReaction_Dead::AllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	// [NOTE] Dead reaction is terminal and cannot be interrupted or cancelled.
	return false;
}
