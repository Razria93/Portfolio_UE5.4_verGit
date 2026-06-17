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

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = relationship;
	return result;
}

void UCReaction_Hit::ResolveObservableOverlayExecutionCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
{
	OutDecision = FObservableOverlayExecutionDecision();

	const bool bIsHitReaction = IsIncomingReactionType(InQuery.DecisionQuery, EReactionType::Hit);
	if (!bIsHitReaction)
	{
		// Hit only.
		OutDecision.Decision = EExecutionDecision::Reject;
		return;
	}

	const bool bHasGuardOverlay = InQuery.DecisionQuery.Snapshot.ObservableOverlay.Guard.HasGuardOverlay();
	if (bHasGuardOverlay)
	{
		// GuardOverlay Case: interrupt Guard lifecycle before Hit.
		OutDecision.Decision = EExecutionDecision::Accept;
		OutDecision.Handlings.AddUnique(EObservableOverlayHandling::ClearGuardState);
		return;
	}

	// Another Case: No overlay cleanup.
	OutDecision.Decision = EExecutionDecision::Accept;
}
