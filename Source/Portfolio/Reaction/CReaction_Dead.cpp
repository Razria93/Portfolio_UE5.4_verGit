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

void UCReaction_Dead::ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
{
	OutDecision = FObservableOverlayExecutionDecision();

	const bool bIsDeadReaction = IsIncomingReactionType(InQuery.DecisionQuery, EReactionType::Dead);
	if (!bIsDeadReaction)
	{
		// Dead only.
		OutDecision.Decision = EExecutionDecision::Reject;
		return;
	}

	const bool bHasGuardState = InQuery.DecisionQuery.Snapshot.ObservableOverlay.Guard.HasGuardRuntimeState();
	if (bHasGuardState)
	{
		// GuardState Case: clear Guard before Dead.
		OutDecision.Decision = EExecutionDecision::Accept;
		OutDecision.Handlings.AddUnique(EObservableOverlayHandling::ClearGuardState);
		return;
	}

	// Another Case: No overlay cleanup.
	OutDecision.Decision = EExecutionDecision::Accept;
}

bool UCReaction_Dead::AllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	// Dead reaction is terminal and cannot be interrupted.
	return false;
}
