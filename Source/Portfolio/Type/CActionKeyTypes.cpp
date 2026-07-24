#include "Type/CActionKeyTypes.h"

bool FActionDataKey::IsValidMinimal() const
{
	return ActionType != EActionType::None
		&& ActionType != EActionType::Max;
}
