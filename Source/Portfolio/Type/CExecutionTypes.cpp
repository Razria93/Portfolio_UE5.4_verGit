#include "Type/CExecutionTypes.h"

#include "Action/CAction.h"
#include "Reaction/CReaction.h"

// Runtime State

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
