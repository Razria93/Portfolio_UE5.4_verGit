#include "Reaction/CReaction_Hit.h"

#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

// Decision

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

// Observable Overlay

void UCReaction_Hit::ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
{
	OutDecision = FObservableOverlayExecutionDecision();

	const bool bIsHitReaction = IsIncomingReactionType(InQuery.DecisionQuery, EReactionType::Hit);
	if (!bIsHitReaction)
	{
		// Reject non-Hit overlay queries.
		OutDecision.Decision = EExecutionDecision::Reject;
		return;
	}

	const bool bHasGuardState = InQuery.DecisionQuery.Snapshot.ObservableOverlay.Guard.HasGuardRuntimeState();
	if (bHasGuardState)
	{
		// Hit clears an active Guard state before it starts.
		OutDecision.Decision = EExecutionDecision::Accept;
		OutDecision.Handlings.AddUnique(EObservableOverlayHandling::ClearGuardState);
		return;
	}

	// Hit can start without overlay cleanup.
	OutDecision.Decision = EExecutionDecision::Accept;
}
