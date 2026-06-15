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
	if (IsValid(OwnerActionComp_Injected))
	{
		OwnerActionComp_Injected->NotifyGuardInterrupted(InStopReason);
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

	if (!CanResolveIndependentRelationship(InQuery))
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
	result.Relationship = EExecutionRelationship::Independent;
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
