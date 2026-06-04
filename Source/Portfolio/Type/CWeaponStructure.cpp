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

namespace
{
	bool IsValidExecutionDomain(EExecutionDomain InDomain)
	{
		return InDomain != EExecutionDomain::None && InDomain != EExecutionDomain::Max;
	}

	bool IsValidActionType(EActionType InType)
	{
		return InType != EActionType::None && InType != EActionType::Max;
	}

	bool IsValidReactionType(EReactionType InType)
	{
		return InType != EReactionType::None && InType != EReactionType::Max;
	}

	bool MatchActionType(EActionType InPattern, EActionType InValue)
	{
		return InPattern == EActionType::All || InPattern == InValue;
	}

	bool MatchIndex(int32 InPattern, int32 InValue)
	{
		return InPattern == INDEX_NONE || InPattern == InValue;
	}

	bool MatchReactionType(EReactionType InPattern, EReactionType InValue)
	{
		return InPattern == EReactionType::All || InPattern == InValue;
	}
}

bool FExecutionInterventionParticipantFilter::IsValidMinimal() const
{
	if (!IsValidExecutionDomain(Domain)) return false;

	switch (Domain)
	{
	case EExecutionDomain::Action:
		return IsValidActionType(ActionType);

	case EExecutionDomain::Reaction:
		return IsValidReactionType(ReactionType);

	default:
		return false;
	}
}

bool FExecutionInterventionParticipantFilter::MatchesAction(EActionType InActionType, int32 InIndex) const
{
	return IsValidMinimal()
		&& Domain == EExecutionDomain::Action
		&& MatchActionType(ActionType, InActionType)
		&& MatchIndex(Index, InIndex);
}

bool FExecutionInterventionParticipantFilter::MatchesReaction(EReactionType InReactionType) const
{
	return IsValidMinimal()
		&& Domain == EExecutionDomain::Reaction
		&& MatchReactionType(ReactionType, InReactionType);
}

bool FExecutionInterventionParticipantFilter::MatchesParticipant(const FExecutionParticipant& InParticipant) const
{
	if (!IsValidMinimal()) return false;
	if (!InParticipant.IsValidMinimal()) return false;
	if (Domain != InParticipant.ParticipantDomain) return false;

	switch (Domain)
	{
	case EExecutionDomain::Action:
		return InParticipant.IsActionParticipant()
			&& MatchesAction(InParticipant.GetActionContext().ActionDataKey.ActionType, InParticipant.GetActionContext().ActionDataKey.ActionIndex);

	case EExecutionDomain::Reaction:
	{
		return InParticipant.IsReactionParticipant()
			&& MatchesReaction(InParticipant.GetReactionContext().ReactionDataKey.ReactionType);
	}

	default:
		return false;
	}
}
