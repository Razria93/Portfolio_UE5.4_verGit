#include "Type/CReactionDataTypes.h"

#include "Reaction/CReaction.h"

bool FReactionData::IsValidMinimal() const
{
	return ReactionDataKey.IsValidMinimal()
		&& ReactionDataKey.ReactionType != EReactionType::All
		&& IsValid(ReactionExecutorKey)
		&& IsValid(Montage);
}

bool FReactionExecutionContext::IsValidMinimal() const
{
	return ReactionDataKey.IsValidMinimal()
		&& ReactionData.IsValidMinimal()
		&& IsValid(ReactionExecutor);
}
