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
			OwnerActionComp_Injected->NotifyGuardStarted();
		}
		else if (InData.ActionDataKey.ActionIndex == 2)
		{
			OwnerActionComp_Injected->NotifyGuardEnded();
		}
	}

	return true;
}

void UCAction_Guard::Stop(EActionStopReason InStopReason)
{
	if (ActiveDataKey_Cached.ActionIndex == 1 && IsValid(OwnerActionComp_Injected))
	{
		OwnerActionComp_Injected->NotifyGuardEnded();
	}

	Super::Stop(InStopReason);
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
