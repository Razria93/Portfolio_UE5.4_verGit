#include "Type/CActionOrchestrationStructure.h"
#include "Action/CAction.h"

bool FActionExecutionContext::IsValidMinimal() const
{
	return ActionDataKey.IsValidMinimal()
		&& ActionData.IsValidMinimal()
		&& IsValid(ActionExecutor);
}
