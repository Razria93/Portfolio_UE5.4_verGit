#include "Action/CAction_ComboAttack.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"
#include "Component/CActionComponent.h"

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

	if (!IsValid(WeaponComp_Cached))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed))
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

bool UCAction_ComboAttack::ReserveChain(const FActionData& InData)
{
	if (!CanReserveChain(InData)) return false;

	ReservingChainData = InData;
	bHasReservingChain = true;
	bReserveChainWindowOpened = false;

	// FLog::Log(TEXT("[ComboAttack] Chain input buffered."));

	return true;
}

void UCAction_ComboAttack::ClearRuntime()
{
	Super::ClearRuntime();

	ReservingChainData = FActionData();
	bHasReservingChain = false;
	bReserveChainWindowOpened = false;
}

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

void UCAction_ComboAttack::ConsumeChain()
{
	const FActionData nextData = ReservingChainData;

	if (!CanConsumeChain(nextData))
	{
		// FLog::Log(TEXT("[ComboAttack] Failed to consume chain."));
		return;
	}

	ReservingChainData = FActionData();
	bHasReservingChain = false;
	bReserveChainWindowOpened = false;

	ActiveDataKey_Cached = nextData.ActionDataKey;
	ActiveData_Cached = nextData;
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

	if (IsValid(OwnerActionComp_Injected))
	{
		// Sync with ActionComponent
		if (!OwnerActionComp_Injected->HandleApplyActionConsumed(this, nextData))
		{
			Stop(EActionStopReason::Ignored);
			return;
		}
	}

	const FActionFeedbackRequest feedbackRequest = BuildFeedbackRequest(EActionFeedbackTiming::Chain);
	PlayFeedbackRequest(feedbackRequest);
	EmitActionEvent(EActionEventType::ActionChained, ActiveDataKey_Cached.ActionIndex);

	// FLog::Log(FString::Printf(TEXT("[ComboAttack] Consume Chain. ActionIndex = %d"), ActiveDataKey_Cached.ActionIndex));
}

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
	if (incomingKey.ActionIndex != activeKey.ActionIndex + 1) return false;

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
	if (incomingKey.ActionIndex != ActiveDataKey_Cached.ActionIndex + 1) return false;

	return true;
}

bool UCAction_ComboAttack::CanConsumeChain(const FActionData& InData) const
{
	if (!InData.IsValidMinimal()) return false;
	if (!ReservingChainData.IsValidMinimal()) return false;
	if (!(InData.ActionDataKey == ReservingChainData.ActionDataKey)) return false;

	if (!bIsActive) return false;
	if (!bHasReservingChain) return false;

	if (!IsValid(OwnerActionComp_Injected)) return false;
	if (!OwnerActionComp_Injected->CanCommitChain(this, InData)) return false;

	return true;
}
