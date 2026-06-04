#include "Type/CReactionOrchestrationStructure.h"
#include "Reaction/CReaction.h"

bool FReactionExecutionContext::IsValidMinimal() const
{
	return ReactionDataKey.IsValidMinimal()
		&& ReactionData.IsValidMinimal()
		&& IsValid(ReactionExecutor);
}
