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
	if (ActiveDataKey_Cached.ActionIndex == 2 && IsValid(OwnerActionComp_Injected))
	{
		OwnerActionComp_Injected->NotifyGuardEnded();
	}

	Super::Complete();
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
