#include "Action/CAction_Guard.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Component/CWeaponComponent.h"

// CAction_Guard only owns Guard In/Out transition actions.
// Guard Hold is an idle overlay state owned by UCDefenseComponent.
bool UCAction_Guard::Start(const FActionData& InData)
{
	const bool bStarted = Super::Start(InData);
	if (!bStarted) return false;

	if (IsValid(OwnerActionComp_Injected))
	{
		const EGuardActionPhase guardPhase = ResolveGuardActionPhase(InData.ActionDataKey);

		switch (guardPhase)
		{
		case EGuardActionPhase::In:
			OwnerActionComp_Injected->ApplyOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::GuardInStarted));
			break;

		case EGuardActionPhase::Out:
			OwnerActionComp_Injected->ApplyOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::GuardOutStarted));
			break;

		default:
			break;
		}
	}

	return true;
}

void UCAction_Guard::Interrupt(const FExecutionInterventionDirective& InDirective)
{
	const EGuardActionPhase activeGuardPhase = ResolveGuardActionPhase(ActiveDataKey_Cached);

	if (IsValid(OwnerActionComp_Injected))
	{
		switch (activeGuardPhase)
		{
		case EGuardActionPhase::In:
		{
			const bool bIsParryIncoming =
				InDirective.IncomingPart.IsReactionParticipant()
				&& InDirective.IncomingPart.GetReactionContext().ReactionDataKey.ReactionType == EReactionType::Parry;

			const bool bIsBlockHitIncoming =
				InDirective.IncomingPart.IsReactionParticipant()
				&& InDirective.IncomingPart.GetReactionContext().ReactionDataKey.ReactionType == EReactionType::BlockHit;

			PrintGuardInterventionDebugInfo(bIsParryIncoming ? TEXT("InterruptGuardInParry") : TEXT("InterruptGuardIn"), activeGuardPhase, InDirective.IncomingPart, bIsBlockHitIncoming);

			if (!bIsBlockHitIncoming)
			{
				ClearDeferredGuardActions();
				ClearGuardState();
			}
			break;
		}

		case EGuardActionPhase::Out:
		{
			const bool bIsGuardInIncoming =
				InDirective.IncomingPart.IsActionParticipant()
				&& ResolveGuardActionPhase(InDirective.IncomingPart.GetActionContext().ActionDataKey) == EGuardActionPhase::In;

			PrintGuardInterventionDebugInfo(TEXT("InterruptGuardOut"), activeGuardPhase, InDirective.IncomingPart, bIsGuardInIncoming);

			if (!bIsGuardInIncoming)
			{
				ClearDeferredGuardActions();
				ClearGuardState();
			}
			break;
		}

		default:
			break;
		}
	}

	Super::Interrupt(InDirective);
}

void UCAction_Guard::Complete()
{
	const EGuardActionPhase activeGuardPhase = ResolveGuardActionPhase(ActiveDataKey_Cached);

	Super::Complete();

	if (!IsValid(OwnerActionComp_Injected)) return;

	switch (activeGuardPhase)
	{
	case EGuardActionPhase::In:
		OwnerActionComp_Injected->ConsumeDeferredAction(EDeferredActionConsumeKey::AfterGuardInAction);
		break;

	case EGuardActionPhase::Out:
		OwnerActionComp_Injected->ApplyOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::GuardLifecycleCompleted));
		break;

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

	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();
	const EGuardActionPhase incomingGuardPhase = ResolveGuardActionPhase(incomingContext.ActionDataKey);

	if (incomingGuardPhase != EGuardActionPhase::In && incomingGuardPhase != EGuardActionPhase::Out)
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	const FGuardObservableOverlaySnapshot& guardOverlaySnapshot = InQuery.Snapshot.ObservableOverlay.Guard;
	if (incomingGuardPhase == EGuardActionPhase::Out && !guardOverlaySnapshot.HasGuardRuntimeState())
	{
		FLog::Log(TEXT("[GuardOut] Ignored stale release."));

		result.Decision = EExecutionDecision::Ignore;
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

	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();

	// Guard defer only applies to Guard-Out candidates.
	const EGuardActionPhase incomingGuardPhase = ResolveGuardActionPhase(incomingContext.ActionDataKey);
	if (incomingGuardPhase != EGuardActionPhase::Out) return false;
	
	if (!InQuery.HasActivePart()) return false;

	// Case 1. Guard-In -> Guard-Out
	if (InQuery.ActivePart.IsActionParticipant())
	{
		const FActionExecutionContext& activeContext = InQuery.ActivePart.GetActionContext();
		const EGuardActionPhase activeGuardPhase = ResolveGuardActionPhase(activeContext.ActionDataKey);

		if (activeGuardPhase == EGuardActionPhase::In)
		{
			OutConsumeKey = EDeferredActionConsumeKey::AfterGuardInAction;
			return true;
		}
	}

	// Case 2. Guard-Block -> Guard-Out
	if (InQuery.ActivePart.IsReactionParticipant())
	{
		const FReactionExecutionContext& activeContext = InQuery.ActivePart.GetReactionContext();

		if (activeContext.ReactionDataKey.ReactionType == EReactionType::BlockHit)
		{
			OutConsumeKey = EDeferredActionConsumeKey::AfterGuardBlockReaction;
			return true;
		}
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

	// CAction_Guard does not execute Guard Hold / Hit / Parry phases.
	OutDecision.Decision = EExecutionDecision::Reject;
}

void UCAction_Guard::HandleSpecificNotifyCommand(EActionNotifyCommand InCommand)
{
	if (!IsValid(OwnerActionComp_Injected)) return;

	switch (InCommand)
	{
	case EActionNotifyCommand::SwitchToGuard:
		OwnerActionComp_Injected->ApplyOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::SwitchToGuard));
		return;

	case EActionNotifyCommand::AllowGuardStart:
		OwnerActionComp_Injected->ApplyOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::AllowGuardStart));
		return;

	default:
		break;
	}
}

bool UCAction_Guard::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!InQuery.IncomingPart.IsActionParticipant()) return false;
	if (!InQuery.ActivePart.IsActionParticipant()) return false;
	if (!IsIncomingActionType(InQuery, EActionType::Guard)) return false;

	if (Super::WantIntervention(InQuery)) return true;

	const FActionExecutionContext& activeContext = InQuery.ActivePart.GetActionContext();
	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();
	const FGuardObservableOverlaySnapshot& guardOverlaySnapshot = InQuery.Snapshot.ObservableOverlay.Guard;

	const EGuardActionPhase activeGuardPhase = ResolveGuardActionPhase(activeContext.ActionDataKey);
	const EGuardActionPhase incomingGuardPhase = ResolveGuardActionPhase(incomingContext.ActionDataKey);

	if (activeGuardPhase == EGuardActionPhase::Out && incomingGuardPhase == EGuardActionPhase::In && guardOverlaySnapshot.bCanStartGuard)
	{
		return true;
	}

	return false;
}

bool UCAction_Guard::AllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!InQuery.ActivePart.IsActionParticipant()) return false;

	if (Super::AllowIntervention(InQuery)) return true;

	const FActionExecutionContext& activeContext = InQuery.ActivePart.GetActionContext();
	const FGuardObservableOverlaySnapshot& guardOverlaySnapshot = InQuery.Snapshot.ObservableOverlay.Guard;

	const EGuardActionPhase activeGuardPhase = ResolveGuardActionPhase(activeContext.ActionDataKey);

	if (InQuery.IncomingPart.IsActionParticipant())
	{
		const FActionExecutionContext& incomingActionContext = InQuery.IncomingPart.GetActionContext();
		const EGuardActionPhase incomingGuardPhase = ResolveGuardActionPhase(incomingActionContext.ActionDataKey);

		if (activeGuardPhase == EGuardActionPhase::Out && incomingGuardPhase == EGuardActionPhase::In && guardOverlaySnapshot.bCanStartGuard)
		{
			PrintGuardInterventionDebugInfo(TEXT("AllowGuardIn"), activeGuardPhase, InQuery.IncomingPart, true);
			return true;
		}
	}

	if (InQuery.IncomingPart.IsReactionParticipant())
	{
		const FReactionExecutionContext& incomingReactionContext = InQuery.IncomingPart.GetReactionContext();
		const EReactionType incomingReactionType = incomingReactionContext.ReactionDataKey.ReactionType;

		if (activeGuardPhase == EGuardActionPhase::In && incomingReactionType == EReactionType::Parry)
		{
			PrintGuardInterventionDebugInfo(TEXT("AllowParry"), activeGuardPhase, InQuery.IncomingPart, true);
			return true;
		}

		if (activeGuardPhase == EGuardActionPhase::In && incomingReactionType == EReactionType::BlockHit)
		{
			PrintGuardInterventionDebugInfo(TEXT("AllowBlockHit"), activeGuardPhase, InQuery.IncomingPart, true);
			return true;
		}
	}

	return false;
}

void UCAction_Guard::ClearDeferredGuardActions() const
{
	if (!IsValid(OwnerActionComp_Injected)) return;

	OwnerActionComp_Injected->ClearDeferredActions(EDeferredActionConsumeKey::AfterGuardInAction);
	OwnerActionComp_Injected->ClearDeferredActions(EDeferredActionConsumeKey::AfterGuardBlockReaction);
}

void UCAction_Guard::ClearGuardState() const
{
	if (!IsValid(OwnerActionComp_Injected)) return;

	OwnerActionComp_Injected->ApplyOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::GuardLifecycleInterrupted));
}

void UCAction_Guard::PrintGuardInterventionDebugInfo(const FString& InStage, EGuardActionPhase InActiveGuardPhase, const FExecutionParticipant& InIncomingPart, bool bKeepGuardState) const
{
	FString incomingText = TEXT("Invalid");

	if (InIncomingPart.IsActionParticipant())
	{
		const FActionExecutionContext& incomingContext = InIncomingPart.GetActionContext();
		incomingText = FString::Printf(
			TEXT("Action:%s|Phase:%s"),
			*UEnum::GetValueAsString(incomingContext.ActionDataKey.ActionType),
			*UEnum::GetValueAsString(ResolveGuardActionPhase(incomingContext.ActionDataKey)));
	}
	else if (InIncomingPart.IsReactionParticipant())
	{
		const FReactionExecutionContext& incomingContext = InIncomingPart.GetReactionContext();
		incomingText = FString::Printf(
			TEXT("Reaction:%s"),
			*UEnum::GetValueAsString(incomingContext.ReactionDataKey.ReactionType));
	}

	FLog::Log(FString::Printf(
		TEXT("[GuardIntervention] Stage=%s | ActivePhase=%s | Incoming=%s | KeepGuardState=%s"),
		*InStage,
		*UEnum::GetValueAsString(InActiveGuardPhase),
		*incomingText,
		bKeepGuardState ? TEXT("true") : TEXT("false")));
}
