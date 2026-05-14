#include "Type/CWeaponStructure.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"

#include "Component/CMovementComponent.h"

#include "Action/CAction.h"
#include "Reaction/CReaction.h"

bool FActionDataKey::IsValidTypeOnlyKey() const
{
	return ActionType != EActionType::None
		&& ActionType != EActionType::All
		&& ActionType != EActionType::Max;
}

bool FActionDataKey::IsValidExactKey() const
{
	return ActionType != EActionType::None
		&& ActionType != EActionType::Max;
}

bool FActionData::IsValidMinimal() const
{
	return ActionDataKey.IsValidExactKey()
		&& IsValid(ActionExecutorKey.Get())
		&& IsValid(Montage);
}

bool FOverlapContext::IsValidMinimal() const
{
	return IsValid(OwnerActor) && IsValid(DamageCauser) && IsValid(OtherActor);
}

bool FReactionData::IsValidMinimal() const
{
	return ReactionDataKey.ReactionType != EReactionType::None
		&& ReactionDataKey.ReactionType != EReactionType::All
		&& ReactionDataKey.ReactionType != EReactionType::Max
		&& IsValid(ReactionExecutorKey)
		&& IsValid(Montage);
}

bool FReactionQueryContext::IsValidMinimal() const
{
	return IsValid(ActiveReactionExecutor) && IsValid(IncomingReactionExecutor);
}

bool FReactionContext::IsValidMinimal() const
{
	return ReactionData.IsValidMinimal() && IsValid(ReactionExecutor);
}
