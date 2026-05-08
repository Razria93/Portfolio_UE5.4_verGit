#include "Notify/CAnimNotify_ActionBase.h"
#include "ProjectGlobal.h"

#include "Action/CAction.h"

#include "Type/CWeaponStructure.h"

UCAnimNotify_ActionBase::UCAnimNotify_ActionBase()
{
}

bool UCAnimNotify_ActionBase::CanProcessActionNotify(const UCAction* InCurrentAction) const
{
	if (!IsValid(InCurrentAction)) return false;

	if (TriggerActionType == EActionType::Max)
	{
		FLog::Log(TEXT("[CAnimNotify_ActionBase] TriggerActionType is not configured."));
		return false;
	}

	const FActionContext actionContext = InCurrentAction->GetActionContext();

	if (TriggerActionType != EActionType::All && actionContext.ActionType != TriggerActionType)
	{
		return false;
	}

	if (TriggerActionIndex != INDEX_NONE && actionContext.ActionIndex != TriggerActionIndex)
	{
		return false;
	}

	return true;
}

