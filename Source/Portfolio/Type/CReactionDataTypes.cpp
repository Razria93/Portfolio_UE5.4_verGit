#include "Type/CReactionDataTypes.h"

#include "Reaction/CReaction.h"

bool FReactionDataKey::IsValidMinimal() const
{
	return ReactionType != EReactionType::None
		&& ReactionType != EReactionType::Max
		&& DamageSpecKey.IsValidMinimal();
}

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
