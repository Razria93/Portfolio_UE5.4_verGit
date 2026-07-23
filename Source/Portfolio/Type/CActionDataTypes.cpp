#include "Type/CActionDataTypes.h"

#include "Action/CAction.h"

bool FActionData::IsValidMinimal() const
{
	return ActionDataKey.IsValidMinimal()
		&& IsValid(ActionExecutorKey.Get())
		&& IsValid(Montage);
}

bool FActionExecutionContext::IsValidMinimal() const
{
	return ActionDataKey.IsValidMinimal()
		&& ActionData.IsValidMinimal()
		&& IsValid(ActionExecutor);
}
