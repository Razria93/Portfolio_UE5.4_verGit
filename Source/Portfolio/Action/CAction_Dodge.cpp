#include "Action/CAction_Dodge.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

FExecutionDecisionResult UCAction_Dodge::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!IsIncomingActionType(InQuery, EActionType::Dodge))
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

void UCAction_Dodge::ResolveObservableOverlayExecutionCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
{
	OutDecision = FObservableOverlayExecutionDecision();

	const bool bIsDodge = IsIncomingActionType(InQuery.DecisionQuery, EActionType::Dodge);
	if (!bIsDodge)
	{
		// Dodge only.
		OutDecision.Decision = EExecutionDecision::Reject;
		return;
	}

	// GuardOverlay Case: Clear Guard before Dodge.
	const bool bHasGuardOverlay = InQuery.DecisionQuery.Snapshot.ObservableOverlay.Guard.HasGuardOverlay();
	if (bHasGuardOverlay)
	{
		OutDecision.Decision = EExecutionDecision::Accept;
		OutDecision.Handlings.AddUnique(EObservableOverlayHandling::ClearGuardOverlay);
		return;
	}

	// Another Case: No overlay cleanup.
	OutDecision.Decision = EExecutionDecision::Accept;
}

bool UCAction_Dodge::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!IsIncomingActionType(InQuery, EActionType::Dodge)) return false;

	if (Super::WantIntervention(InQuery)) return true;

	return InQuery.StopReason == EExecutionStopReason::Interrupted;
}
