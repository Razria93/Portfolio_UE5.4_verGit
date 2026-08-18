#include "Action/CAction_ComboAttack.h"

#include "Component/CWeaponComponent.h"
#include "Component/CActionComponent.h"

#include "GameFramework/Character.h"

// Decision

FExecutionDecisionResult UCAction_ComboAttack::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!IsIncomingActionType(InQuery, EActionType::ComboAttack))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!IsValid(WeaponComp_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (WeaponComp_Injected->CheckCurrentWeaponType(EWeaponType::Unarmed))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (CanResolveChain(InQuery))
	{
		result.Decision = EExecutionDecision::Accept;
		result.Relationship = EExecutionRelationship::Sequential;
		return result;
	}

	if (!CanResolveIndependentRelationship(InQuery))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = EExecutionRelationship::Independent;
	return result;
}

// Chain Reservation

bool UCAction_ComboAttack::ReserveChain(const FActionData& InData, const uint32 InActionRequestSerial)
{
	if (!CanReserveChain(InData)) return false;

	ReservingChainData = InData;
	ReservingChainActionRequestSerial = InActionRequestSerial;
	bHasReservingChain = true;
	bReserveChainWindowOpened = false;

	return true;
}

// Lifecycle

void UCAction_ComboAttack::ClearRuntime()
{
	Super::ClearRuntime();

	ReservingChainData = FActionData();
	ReservingChainActionRequestSerial = 0;
	bHasReservingChain = false;
	bReserveChainWindowOpened = false;
}

// Notify

void UCAction_ComboAttack::HandleSpecificNotifyCommand(EActionNotifyCommand InCommand)
{
	switch (InCommand)
	{
	case EActionNotifyCommand::OpenReserveChainWindow:
		OpenReserveChainWindow();
		return;

	case EActionNotifyCommand::CloseReserveChainWindow:
		CloseReserveChainWindow();
		return;

	case EActionNotifyCommand::ConsumeChain:
		ConsumeChain();
		return;

	default:
		return;
	}
}

// Chain Window

void UCAction_ComboAttack::OpenReserveChainWindow()
{
	if (!bIsActive) return;

	bReserveChainWindowOpened = true;

	EmitActionEvent(EActionEventType::ReserveChainWindowOpened, ActiveDataKey_Cached.ActionIndex);
}

void UCAction_ComboAttack::CloseReserveChainWindow()
{
	if (!bReserveChainWindowOpened) return;

	bReserveChainWindowOpened = false;

	EmitActionEvent(EActionEventType::ReserveChainWindowClosed, ActiveDataKey_Cached.ActionIndex);
}

// Chain Consume

void UCAction_ComboAttack::ConsumeChain()
{
	const FActionData nextData = ReservingChainData;
	const uint32 nextActionRequestSerial = ReservingChainActionRequestSerial;

	if (!CanConsumeChain(nextData))
	{
		return;
	}

	ReservingChainData = FActionData();
	ReservingChainActionRequestSerial = 0;
	bHasReservingChain = false;
	bReserveChainWindowOpened = false;

	ActiveDataKey_Cached = nextData.ActionDataKey;
	ActiveData_Cached = nextData;
	ActionRequestSerial_Cached = nextActionRequestSerial;
	ActiveMontage_Cached = nextData.Montage;
	LastStopReason_Cached = EActionStopReason::None;

	if (!PlayMontage(nextData))
	{
		Stop(EActionStopReason::Ignored);
		return;
	}

	if (!BindMontageEndDelegate())
	{
		Stop(EActionStopReason::Ignored);
		return;
	}

	if (IsValid(ActionComp_Injected))
	{
		// Keep the owning action component synchronized with the consumed chain.
		if (!ActionComp_Injected->HandleApplyActionConsumed(this, nextData, nextActionRequestSerial))
		{
			Stop(EActionStopReason::Ignored);
			return;
		}
	}

	const FActionFeedbackRequest feedbackRequest = BuildFeedbackRequest(EActionFeedbackTiming::Chain);
	PlayFeedbackRequest(feedbackRequest);
	EmitActionEvent(EActionEventType::ActionStarted, ActiveDataKey_Cached.ActionIndex);
	EmitActionEvent(EActionEventType::ActionChained, ActiveDataKey_Cached.ActionIndex);
}

// Chain Query

bool UCAction_ComboAttack::CanResolveChain(const FExecutionDecisionQuery& InQuery) const
{
	if (!InQuery.Snapshot.IsInAction()) return false;
	if (!InQuery.IncomingPart.IsActionParticipant()) return false;
	if (!InQuery.ActivePart.IsActionParticipant()) return false;

	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();
	const FActionExecutionContext& activeContext = InQuery.ActivePart.GetActionContext();

	const FActionDataKey& incomingKey = incomingContext.ActionDataKey;
	const FActionDataKey& activeKey = activeContext.ActionDataKey;

	if (activeKey.ActionType != EActionType::ComboAttack) return false;
	if (incomingKey.ActionType != activeKey.ActionType) return false;
	if (incomingKey.ActionIndex != activeKey.ActionIndex + CActionIndexConstants::NextSequentialActionOffset) return false;

	return true;
}

bool UCAction_ComboAttack::CanReserveChain(const FActionData& InData) const
{
	if (!InData.IsValidMinimal()) return false;

	if (!bIsActive) return false;
	if (bHasReservingChain) return false;
	if (!bReserveChainWindowOpened) return false;

	const FActionDataKey& incomingKey = InData.ActionDataKey;

	if (incomingKey.ActionType != ActiveDataKey_Cached.ActionType) return false;
	if (incomingKey.ActionIndex != ActiveDataKey_Cached.ActionIndex + CActionIndexConstants::NextSequentialActionOffset) return false;

	return true;
}

bool UCAction_ComboAttack::CanConsumeChain(const FActionData& InData) const
{
	if (!InData.IsValidMinimal()) return false;
	if (!ReservingChainData.IsValidMinimal()) return false;
	if (!(InData.ActionDataKey == ReservingChainData.ActionDataKey)) return false;

	if (!bIsActive) return false;
	if (!bHasReservingChain) return false;

	if (!IsValid(ActionComp_Injected)) return false;
	if (!ActionComp_Injected->CanCommitChain(this, InData)) return false;

	return true;
}
