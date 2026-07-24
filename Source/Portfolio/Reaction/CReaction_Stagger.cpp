#include "Reaction/CReaction_Stagger.h"

#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

FExecutionDecisionResult UCReaction_Stagger::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!IsIncomingReactionType(InQuery, EReactionType::Stagger))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (InQuery.Snapshot.IsDead())
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!CanResolveExclusiveRelationship(InQuery))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = EExecutionRelationship::Exclusive;
	return result;
}

bool UCReaction_Stagger::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!IsIncomingReactionType(InQuery, EReactionType::Stagger)) return false;

	if (Super::WantIntervention(InQuery)) return true;

	return InQuery.ActivePart.IsActionParticipant() || InQuery.ActivePart.IsReactionParticipant();
}

void UCReaction_Stagger::ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
{
	OutDecision = FObservableOverlayExecutionDecision();

	const bool bIsStaggerReaction = IsIncomingReactionType(InQuery.DecisionQuery, EReactionType::Stagger);
	if (!bIsStaggerReaction)
	{
		// Stagger only.
		OutDecision.Decision = EExecutionDecision::Reject;
		return;
	}

	const bool bHasGuardState = InQuery.DecisionQuery.Snapshot.ObservableOverlay.Guard.HasGuardRuntimeState();
	if (bHasGuardState)
	{
		// GuardState Case: clear Guard before Stagger.
		OutDecision.Decision = EExecutionDecision::Accept;
		OutDecision.Handlings.AddUnique(EObservableOverlayHandling::ClearGuardState);
		return;
	}

	// Another Case: No overlay cleanup.
	OutDecision.Decision = EExecutionDecision::Accept;
}
