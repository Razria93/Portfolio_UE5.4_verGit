#include "Notify/CAnimNotifyState.h"
#include "ProjectGlobal.h"

#include "Action/CAction.h"

#include "Type/CWeaponStructure.h"

UCAnimNotifyState::UCAnimNotifyState()
{
}

bool UCAnimNotifyState::CanProcessActionNotify(const UCAction* InCurrentAction) const
{
	if (!IsValid(InCurrentAction)) return false;

	if (TriggerActionType == EActionType::Max)
	{
		FLog::Log(TEXT("[AnimNotify] TriggerActionType is not configured."));
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
