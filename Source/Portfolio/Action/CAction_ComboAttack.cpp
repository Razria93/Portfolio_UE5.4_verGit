#include "Action/CAction_ComboAttack.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"
#include "Component/CActionComponent.h"

void UCAction_ComboAttack::InitializeAction(ACharacter* InOwnerCharacter, UCActionComponent* InOwnerActionComp)
{
	Super::InitializeAction(InOwnerCharacter, InOwnerActionComp);

	ClearComboRuntime();
}

EActionLocalLevelDecision UCAction_ComboAttack::ResolveLocalLevelDecision(const FActionLocalLevelQuery& InQuery) const
{
	if (!IsValid(OwnerCharacter_Injected)) return EActionLocalLevelDecision::Reject;
	if (!IsValid(WeaponComp_Cached)) return EActionLocalLevelDecision::Reject;

	if (WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed))
	{
		return EActionLocalLevelDecision::Reject;
	}

	const bool bIsIdle = InQuery.ExecutionState == EExecutionState::Idle;
	const bool bIsActiveAction = InQuery.bIsActiveAction;

	if (bIsIdle && !bIsActiveAction)
	{
		return EActionLocalLevelDecision::Start;
	}

	if (!bIsActiveAction)
	{
		return EActionLocalLevelDecision::Reject;
	}

	if (!InQuery.ActiveContext.IsValidMinimal())
	{
		return EActionLocalLevelDecision::Reject;
	}

	const bool bIsEqualAction = InQuery.ActiveContext.ActionDataKey.ActionType == InQuery.IncomingContext.ActionDataKey.ActionType;

	if (bIsEqualAction && bChainWindowOpened)
	{
		return EActionLocalLevelDecision::Chain;
	}

	return EActionLocalLevelDecision::Reject;
}

bool UCAction_ComboAttack::Start(const FActionData& InData)
{
	ClearComboRuntime();

	bool result = Super::Start(InData);

	return result;
}

bool UCAction_ComboAttack::ApplyChain(const FActionData& InData)
{
	if (!CanAcceptChain(InData)) return false;

	PendingChainData_Cached = InData;
	bHasChainedInput = true;
	bChainWindowOpened = false;

	FLog::Log(TEXT("[ComboAttack] Chain input buffered."));

	return true;
}

void UCAction_ComboAttack::Stop(EActionStopReason InReason)
{
	Super::Stop(InReason);

	ClearComboRuntime();
}

void UCAction_ComboAttack::Complete()
{
	Super::Complete();

	ClearComboRuntime();
}

void UCAction_ComboAttack::AdvanceCombo()
{
	if (!CanAdvanceCombo())
	{
		FLog::Log(TEXT("[ComboAttack] No valid pending chain action."));
		return;
	}

	const FActionData nextData = PendingChainData_Cached;

	PendingChainData_Cached = FActionData();
	bHasChainedInput = false;
	bChainWindowOpened = false;

	ActiveData_Cached = nextData;
	ActiveDataKey_Cached = nextData.ActionDataKey;
	ActiveMontage_Cached = nextData.Montage;

	if (!PlayMontage(nextData))
	{
		Stop(EActionStopReason::Ignored);
		return;
	}

	if (IsValid(OwnerActionComp_Injected))
	{
		if (!OwnerActionComp_Injected->HandleActionChained(this, nextData))
		{
			Stop(EActionStopReason::Ignored);
			return;
		}
	}

	RequestFeedback(EActionFeedbackTiming::ActionChain);
	EmitActionEvent(EActionEventType::ActionChained, ActiveDataKey_Cached.ActionIndex);

	FLog::Log(FString::Printf(TEXT("[ComboAttack] Advance combo. ActionIndex = %d"), ActiveDataKey_Cached.ActionIndex));
}

void UCAction_ComboAttack::OpenChainWindow()
{
	if (!bIsActive) return;

	bChainWindowOpened = true;

	EmitActionEvent(EActionEventType::ChainWindowOpened, ActiveDataKey_Cached.ActionIndex);
}

void UCAction_ComboAttack::CloseChainWindow()
{
	if (!bChainWindowOpened) return;

	bChainWindowOpened = false;

	EmitActionEvent(EActionEventType::ChainWindowClosed, ActiveDataKey_Cached.ActionIndex);
}

void UCAction_ComboAttack::ClearComboRuntime()
{
	bChainWindowOpened = false;
	bHasChainedInput = false;
	PendingChainData_Cached = FActionData();
}

bool UCAction_ComboAttack::CanAcceptChain(const FActionData& InData) const
{
	if (!bIsActive) return false;
	if (!bChainWindowOpened) return false;
	if (!InData.IsValidMinimal()) return false;

	const FActionDataKey& incomingKey = InData.ActionDataKey;

	if (incomingKey.ActionType != ActiveDataKey_Cached.ActionType) return false;
	if (incomingKey.ActionIndex <= ActiveDataKey_Cached.ActionIndex) return false;

	return true;
}

bool UCAction_ComboAttack::CanAdvanceCombo() const
{
	return bIsActive && bHasChainedInput && PendingChainData_Cached.IsValidMinimal();
}
