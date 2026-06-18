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
		const EGuardActionPhase guardPhase = ResolveGuardActionPhase(InData.ActionDataKey);

		if (guardPhase == EGuardActionPhase::In)
		{
			OwnerActionComp_Injected->NotifyObservableOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::GuardInStarted));
		}
		else if (guardPhase == EGuardActionPhase::Out)
		{
			OwnerActionComp_Injected->NotifyObservableOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::GuardOutStarted));
		}
	}

	return true;
}

void UCAction_Guard::Stop(EActionStopReason InStopReason)
{
	const EGuardActionPhase activeGuardPhase = ResolveGuardActionPhase(ActiveDataKey_Cached);

	if (IsValid(OwnerActionComp_Injected))
	{
		const bool bIsGuardOutReentryInterruption = activeGuardPhase == EGuardActionPhase::Out && InStopReason == EActionStopReason::Interrupted;

		if (!bIsGuardOutReentryInterruption)
		{
			OwnerActionComp_Injected->ClearDeferredActions(EDeferredActionConsumeKey::GuardInCompleted);
			OwnerActionComp_Injected->NotifyObservableOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::GuardLifecycleInterrupted));
		}
	}

	Super::Stop(InStopReason);
}

void UCAction_Guard::Complete()
{
	const EGuardActionPhase activeGuardPhase = ResolveGuardActionPhase(ActiveDataKey_Cached);

	Super::Complete();

	if (!IsValid(OwnerActionComp_Injected)) return;

	if (activeGuardPhase == EGuardActionPhase::In)
	{
		OwnerActionComp_Injected->ConsumeDeferredAction(EDeferredActionConsumeKey::GuardInCompleted);
	}
	else if (activeGuardPhase == EGuardActionPhase::Out)
	{
		OwnerActionComp_Injected->NotifyObservableOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::GuardLifecycleCompleted));
	}
}

void UCAction_Guard::HandleSpecificNotifyCommand(EActionNotifyCommand InCommand)
{
	if (!IsValid(OwnerActionComp_Injected)) return;

	switch (InCommand)
	{
	case EActionNotifyCommand::SwitchToGuard:
		OwnerActionComp_Injected->NotifyObservableOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::SwitchToGuard));
		return;

	case EActionNotifyCommand::AllowGuardStart:
		OwnerActionComp_Injected->NotifyObservableOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::AllowGuardStart));
		return;

	default:
		break;
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

	const FActionExecutionContext& activeContext = InQuery.ActivePart.GetActionContext();
	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();

	const EGuardActionPhase activeGuardPhase = ResolveGuardActionPhase(activeContext.ActionDataKey);
	const EGuardActionPhase incomingGuardPhase = ResolveGuardActionPhase(incomingContext.ActionDataKey);

	if (activeGuardPhase == EGuardActionPhase::In && incomingGuardPhase == EGuardActionPhase::Out)
	{
		OutConsumeKey = EDeferredActionConsumeKey::GuardInCompleted;
		return true;
	}

	return false;
}

void UCAction_Guard::ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
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
	const EGuardActionPhase incomingGuardPhase = ResolveGuardActionPhase(incomingContext.ActionDataKey);

	const FGuardObservableOverlaySnapshot& guardOverlaySnapshot = InQuery.DecisionQuery.Snapshot.ObservableOverlay.Guard;

	// Case Guard-in: check condition
	if (incomingGuardPhase == EGuardActionPhase::In)
	{
		const bool bCanStartGuardIn = !guardOverlaySnapshot.bIsGuardingPose && guardOverlaySnapshot.bCanStartGuard;
		if (!bCanStartGuardIn)
		{
			// GuardIn blocked by current Guard state.
			OutDecision.Decision = EExecutionDecision::Ignore;
			return;
		}

		OutDecision.Decision = EExecutionDecision::Accept;
		
		// Clear stale Guard overlay before start.
		if (guardOverlaySnapshot.HasGuardOverlay())
		{
			OutDecision.Handlings.AddUnique(EObservableOverlayHandling::ClearGuardOverlay);
		}
		return;
	}

	// Case Guard-out: check condition
	if (incomingGuardPhase == EGuardActionPhase::Out)
	{
		if (!guardOverlaySnapshot.bIsGuardingPose)
		{
			// GuardOut without GuardPose: No-op.
			OutDecision.Decision = EExecutionDecision::Ignore;
			return;
		}

		// GuardOut start clears Guard overlay.
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

	const FActionExecutionContext& activeContext = InQuery.ActivePart.GetActionContext();
	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();
	const FGuardObservableOverlaySnapshot& guardOverlaySnapshot = InQuery.Snapshot.ObservableOverlay.Guard;

	const EGuardActionPhase activeGuardPhase = ResolveGuardActionPhase(activeContext.ActionDataKey);
	const EGuardActionPhase incomingGuardPhase = ResolveGuardActionPhase(incomingContext.ActionDataKey);

	if (activeGuardPhase == EGuardActionPhase::Out && incomingGuardPhase == EGuardActionPhase::In && guardOverlaySnapshot.bCanStartGuard)
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

	const FActionExecutionContext& activeContext = InQuery.ActivePart.GetActionContext();
	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();
	const FGuardObservableOverlaySnapshot& guardOverlaySnapshot = InQuery.Snapshot.ObservableOverlay.Guard;

	const EGuardActionPhase activeGuardPhase = ResolveGuardActionPhase(activeContext.ActionDataKey);
	const EGuardActionPhase incomingGuardPhase = ResolveGuardActionPhase(incomingContext.ActionDataKey);

	if (activeGuardPhase == EGuardActionPhase::Out && incomingGuardPhase == EGuardActionPhase::In && guardOverlaySnapshot.bCanStartGuard)
	{
		return true;
	}

	return Super::AllowIntervention(InQuery);
}
