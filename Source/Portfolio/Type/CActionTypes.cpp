#include "Type/CActionTypes.h"

bool FActionDataKey::IsValidMinimal() const
{
	return ActionType != EActionType::None
		&& ActionType != EActionType::Max;
}
