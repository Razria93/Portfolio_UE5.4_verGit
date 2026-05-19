#include "Type/CWeaponStructure.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"

#include "Component/CMovementComponent.h"

#include "Action/CAction.h"
#include "Reaction/CReaction.h"

bool FActionDataKey::IsValidMinimal() const
{
	return ActionType != EActionType::None
		&& ActionType != EActionType::Max;
}

bool FActionData::IsValidMinimal() const
{
	return ActionDataKey.IsValidMinimal()
		&& IsValid(ActionExecutorKey.Get())
		&& IsValid(Montage);
}

bool FOverlapContext::IsValidMinimal() const
{
	return IsValid(OwnerActor) && IsValid(DamageCauser) && IsValid(OtherActor);
}

bool FReactionDataKey::IsValidMinimal() const
{
	return ReactionType != EReactionType::None
		&& ReactionType != EReactionType::Max
		&& ApplyDamageSpecKey.IsValidMinimal();
}

bool FReactionData::IsValidMinimal() const
{
	return ReactionDataKey.ReactionType != EReactionType::None
		&& ReactionDataKey.ReactionType != EReactionType::All
		&& ReactionDataKey.ReactionType != EReactionType::Max
		&& IsValid(ReactionExecutorKey)
		&& IsValid(Montage);
}

bool FExecutionParticipant::IsValidMinimal() const
{
	if (!bIsValid) return false;

	switch (ParticipantDomain)
	{
	case EExecutionDomain::Action:
		return ActionContext.IsValidMinimal();

	case EExecutionDomain::Reaction:
		return ReactionContext.IsValidMinimal();

	default:
		return false;
	}
}

bool FExecutionParticipant::IsActionParticipant() const
{
	return bIsValid
		&& ParticipantDomain == EExecutionDomain::Action
		&& ActionContext.IsValidMinimal();
}

bool FExecutionParticipant::IsReactionParticipant() const
{
	return bIsValid
		&& ParticipantDomain == EExecutionDomain::Reaction
		&& ReactionContext.IsValidMinimal();
}

const FActionExecutionContext& FExecutionParticipant::GetActionContext() const
{
	check(IsActionParticipant());

	return ActionContext;
}

const FReactionExecutionContext& FExecutionParticipant::GetReactionContext() const
{
	check(IsReactionParticipant());

	return ReactionContext;
}

UObject* FExecutionParticipant::GetExecutor() const
{
	if (IsActionParticipant()) return ActionContext.ActionExecutor;
	if (IsReactionParticipant()) return ReactionContext.ReactionExecutor;

	return nullptr;
}

int32 FExecutionParticipant::GetPriority() const
{
	if (IsActionParticipant()) return ActionContext.ActionData.Priority;
	if (IsReactionParticipant()) return ReactionContext.ReactionData.Priority;

	return 0;
}
