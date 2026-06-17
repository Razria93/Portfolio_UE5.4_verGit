#include "Action/CAction_Guard.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Component/CWeaponComponent.h"

bool UCAction_Guard::Start(const FActionData& InData)
{
	const bool bStarted = Super::Start(InData);
	if (!bStarted) return false;

	if (IsValid(OwnerActionComp_Injected))
	{
		if (InData.ActionDataKey.ActionIndex == 1)
		{
			OwnerActionComp_Injected->NotifyGuardInStarted();
		}
		else if (InData.ActionDataKey.ActionIndex == 2)
		{
			OwnerActionComp_Injected->NotifyGuardOutStarted();
		}
	}

	return true;
}

void UCAction_Guard::Stop(EActionStopReason InStopReason)
{
	const int32 activeActionIndex = ActiveDataKey_Cached.ActionIndex;

	if (IsValid(OwnerActionComp_Injected))
	{
		const bool bIsGuardOutReentryInterruption = activeActionIndex == 2 && InStopReason == EActionStopReason::Interrupted;
		if (!bIsGuardOutReentryInterruption)
		{
			// Clear Deffered Guard Out Action & Clear Guard Rumtime State
			OwnerActionComp_Injected->NotifyGuardInterrupted(InStopReason);
		}
	}

	Super::Stop(InStopReason);
}

void UCAction_Guard::Complete()
{
	const int32 activeActionIndex = ActiveDataKey_Cached.ActionIndex;

	Super::Complete();

	if (!IsValid(OwnerActionComp_Injected)) return;

	if (activeActionIndex == 1)
	{
		OwnerActionComp_Injected->NotifyGuardInCompleted();
	}
	else if (activeActionIndex == 2)
	{
		OwnerActionComp_Injected->NotifyGuardOutCompleted();
	}
}

FExecutionDecisionResult UCAction_Guard::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!IsIncomingActionType(InQuery, EActionType::Guard))
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

	if (WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = relationship;
	return result;
}

bool UCAction_Guard::TryResolveDeferredConsumeKey(const FExecutionDecisionQuery& InQuery, EDeferredActionConsumeKey& OutConsumeKey) const
{
	OutConsumeKey = EDeferredActionConsumeKey::None;

	if (!InQuery.IncomingPart.IsActionParticipant()) return false;

	if (!InQuery.HasActivePart()) return false;
	if (!InQuery.ActivePart.IsActionParticipant()) return false;

	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();
	const FActionExecutionContext& activeContext = InQuery.ActivePart.GetActionContext();

	const bool bIsGuardOutCandidate = incomingContext.ActionDataKey.ActionType == EActionType::Guard && incomingContext.ActionDataKey.ActionIndex == 2;
	const bool bIsActiveGuardIn = activeContext.ActionDataKey.ActionType == EActionType::Guard && activeContext.ActionDataKey.ActionIndex == 1;

	if (bIsGuardOutCandidate && bIsActiveGuardIn)
	{
		OutConsumeKey = EDeferredActionConsumeKey::GuardInCompleted;
		return true;
	}

	return false;
}

void UCAction_Guard::ResolveObservableOverlayExecutionCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
{
	OutDecision = FObservableOverlayExecutionDecision();

	const bool bIsGuard = IsIncomingActionType(InQuery.DecisionQuery, EActionType::Guard);
	if (!bIsGuard)
	{
		// Guard only.
		OutDecision.Decision = EExecutionDecision::Reject;
		return;
	}

	const FActionExecutionContext& incomingContext = InQuery.DecisionQuery.IncomingPart.GetActionContext();
	const FGuardObservableOverlaySnapshot& guardSnapshot = InQuery.DecisionQuery.Snapshot.ObservableOverlay.Guard;
	const bool bIsGuardIn = incomingContext.ActionDataKey.ActionIndex == 1;
	const bool bIsGuardOut = incomingContext.ActionDataKey.ActionIndex == 2;

	if (bIsGuardIn)
	{
		const bool bCanStartGuardIn = !guardSnapshot.bIsGuardingPose && guardSnapshot.bCanStartGuard;
		if (!bCanStartGuardIn)
		{
			// GuardIn blocked by current Guard state.
			OutDecision.Decision = EExecutionDecision::Ignore;
			return;
		}

		// GuardIn Case: clear stale Guard overlay before start.
		OutDecision.Decision = EExecutionDecision::Accept;
		if (guardSnapshot.HasGuardOverlay())
		{
			OutDecision.Handlings.AddUnique(EObservableOverlayHandling::ClearGuardOverlay);
		}
		return;
	}

	if (bIsGuardOut)
	{
		if (!guardSnapshot.bIsGuardingPose)
		{
			// GuardOut without GuardPose: No-op.
			OutDecision.Decision = EExecutionDecision::Ignore;
			return;
		}

		// GuardOut Case: allow; GuardOut start clears Guard overlay.
		OutDecision.Decision = EExecutionDecision::Accept;
		return;
	}

	// Guard Hold / other Guard Case: No overlay cleanup.
	OutDecision.Decision = EExecutionDecision::Accept;
}

bool UCAction_Guard::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!InQuery.IncomingPart.IsActionParticipant()) return false;
	if (!InQuery.ActivePart.IsActionParticipant()) return false;
	if (!IsIncomingActionType(InQuery, EActionType::Guard)) return false;

	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();
	const FActionExecutionContext& activeContext = InQuery.ActivePart.GetActionContext();
	const FGuardObservableOverlaySnapshot& guardSnapshot = InQuery.Snapshot.ObservableOverlay.Guard;

	const bool bIsGuardIn = incomingContext.ActionDataKey.ActionIndex == 1;
	const bool bIsActiveGuardOut = activeContext.ActionDataKey.ActionType == EActionType::Guard && activeContext.ActionDataKey.ActionIndex == 2;

	if (bIsGuardIn && bIsActiveGuardOut && guardSnapshot.bCanStartGuard)
	{
		return true;
	}

	return Super::WantIntervention(InQuery);
}

bool UCAction_Guard::AllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!InQuery.IncomingPart.IsActionParticipant()) return false;
	if (!InQuery.ActivePart.IsActionParticipant()) return false;

	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();
	const FActionExecutionContext& activeContext = InQuery.ActivePart.GetActionContext();
	const FGuardObservableOverlaySnapshot& guardSnapshot = InQuery.Snapshot.ObservableOverlay.Guard;

	const bool bIsIncomingGuardIn = incomingContext.ActionDataKey.ActionType == EActionType::Guard && incomingContext.ActionDataKey.ActionIndex == 1;
	const bool bIsActiveGuardOut = activeContext.ActionDataKey.ActionType == EActionType::Guard && activeContext.ActionDataKey.ActionIndex == 2;

	if (bIsIncomingGuardIn && bIsActiveGuardOut && guardSnapshot.bCanStartGuard)
	{
		return true;
	}

	return Super::AllowIntervention(InQuery);
}
