#include "Reaction/CReaction_Hit.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

FExecutionDecisionResult UCReaction_Hit::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!IsIncomingReactionType(InQuery, EReactionType::Hit))
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

	FLog::Log(FString::Printf(
		TEXT("[UCReaction_Hit::ResolveExecutionDecision] Owner = %s | Relationship = %s"),
		*GetNameSafe(OwnerCharacter_Injected),
		*UEnum::GetValueAsString(relationship)));

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = relationship;
	return result;
}

bool UCReaction_Hit::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!IsIncomingReactionType(InQuery, EReactionType::Hit)) return false;

	// [Condition of WantIntervention] Just Want Interruption. No Cancel.
	bool result = InQuery.StopReason == EExecutionStopReason::Interrupted;

	FLog::Log(FString::Printf(
		TEXT("[UCReaction_Hit::WantIntervention] Owner = %s | StopReason = %s | Want Intervention Result = %s"),
		*GetNameSafe(OwnerCharacter_Injected),
		*UEnum::GetValueAsString(InQuery.StopReason),
		result ? TEXT("true") : TEXT("false")));

	return result;
}
