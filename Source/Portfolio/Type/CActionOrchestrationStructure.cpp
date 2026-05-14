#include "Type/CActionOrchestrationStructure.h"
#include "Action/CAction.h"

bool FActionResolvedContext::IsValidMinimal() const
{
	return ActionDataKey.IsValidExactKey()
		&& ActionData.IsValidMinimal()
		&& IsValid(ActionExecutor);
}
