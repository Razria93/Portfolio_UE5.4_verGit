#include "Type/CExecutionRuleTypes.h"

#include "Type/CExecutionTypes.h"

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
