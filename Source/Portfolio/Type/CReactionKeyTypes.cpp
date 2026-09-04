#include "Type/CReactionKeyTypes.h"

bool FReactionDataKey::IsValidMinimal() const
{
	if (ReactionType == EReactionType::None || ReactionType == EReactionType::Max) return false;
	if (MatchMode == EReactionDataMatchMode::DamageSpec) return DamageSpecKey.IsValidMinimal();
	if (MatchMode == EReactionDataMatchMode::Global) return ReactionIndex == INDEX_NONE;

	return false;
}
