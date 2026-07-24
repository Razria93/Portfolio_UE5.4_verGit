#include "Type/CReactionKeyTypes.h"

bool FReactionDataKey::IsValidMinimal() const
{
	return ReactionType != EReactionType::None
		&& ReactionType != EReactionType::Max
		&& DamageSpecKey.IsValidMinimal();
}
