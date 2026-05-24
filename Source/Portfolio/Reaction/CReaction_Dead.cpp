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

	// if (Super::WantIntervention(InQuery)) return true;

	// [Condition of WantIntervention] Just Want Interruption. No Cancel.
	bool result = InQuery.StopReason == EExecutionStopReason::Interrupted;

	FLog::Log(FString::Printf(
		TEXT("[UCReaction_Dead::WantIntervention] Owner = %s | StopReason = %s | Want Intervention Result = %s"),
		*GetNameSafe(OwnerCharacter_Injected),
		*UEnum::GetValueAsString(InQuery.StopReason),
		result ? TEXT("true") : TEXT("false")));

	return result;
}

bool UCReaction_Dead::AllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	// [NOTE] Dead reaction is terminal and cannot be interrupted or cancelled.
	return false;
}
