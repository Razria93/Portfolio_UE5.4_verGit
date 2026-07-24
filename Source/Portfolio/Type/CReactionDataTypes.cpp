#include "Type/CReactionDataTypes.h"

#include "Reaction/CReaction.h"

bool FReactionData::IsValidMinimal() const
{
	return ReactionDataKey.ReactionType != EReactionType::None
		&& ReactionDataKey.ReactionType != EReactionType::All
		&& ReactionDataKey.ReactionType != EReactionType::Max
		&& IsValid(ReactionExecutorKey)
		&& IsValid(Montage);
}

bool FReactionExecutionContext::IsValidMinimal() const
{
	return ReactionDataKey.IsValidMinimal()
		&& ReactionData.IsValidMinimal()
		&& IsValid(ReactionExecutor);
}
