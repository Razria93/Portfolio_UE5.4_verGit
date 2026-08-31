#include "Reaction/CReaction_ExecutionRecovery.h"

#include "GameFramework/Character.h"

FExecutionDecisionResult UCReaction_ExecutionRecovery::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected)
		|| !IsIncomingReactionType(InQuery, EReactionType::ExecutionRecovery)
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

bool UCReaction_ExecutionRecovery::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	return IsIncomingReactionType(InQuery, EReactionType::ExecutionRecovery)
		&& (InQuery.ActivePart.IsActionParticipant() || InQuery.ActivePart.IsReactionParticipant());
}
