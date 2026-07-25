#include "Action/CAction_Dodge.h"

#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

// Decision

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

// Observable Overlay

void UCAction_Dodge::ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
{
	OutDecision = FObservableOverlayExecutionDecision();

	const bool bIsDodge = IsIncomingActionType(InQuery.DecisionQuery, EActionType::Dodge);
	if (!bIsDodge)
	{
		// Reject non-Dodge overlay queries.
		OutDecision.Decision = EExecutionDecision::Reject;
		return;
	}

	// Dodge clears an active Guard state before it starts.
	const bool bHasGuardState = InQuery.DecisionQuery.Snapshot.ObservableOverlay.Guard.HasGuardRuntimeState();
	if (bHasGuardState)
	{
		OutDecision.Decision = EExecutionDecision::Accept;
		OutDecision.Handlings.AddUnique(EObservableOverlayHandling::ClearGuardState);
		return;
	}

	// Dodge can start without overlay cleanup.
	OutDecision.Decision = EExecutionDecision::Accept;
}

// Intervention

bool UCAction_Dodge::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!IsIncomingActionType(InQuery, EActionType::Dodge)) return false;

	if (Super::WantIntervention(InQuery)) return true;

	return InQuery.StopReason == EExecutionStopReason::Interrupted;
}
