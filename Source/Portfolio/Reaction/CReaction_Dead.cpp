#include "Reaction/CReaction_Dead.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

EExecutionDecision UCReaction_Dead::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	if (!InQuery.IncomingPart.IsReactionParticipant()) return EExecutionDecision::Reject;

	const FReactionExecutionContext& incoming = InQuery.IncomingPart.GetReactionContext();
	if (incoming.ReactionDataKey.ReactionType != EReactionType::Dead) return EExecutionDecision::Reject;

	return EExecutionDecision::Executable;
}

bool UCReaction_Dead::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!InQuery.IncomingPart.IsReactionParticipant()) return false;

	const FReactionExecutionContext& incoming = InQuery.IncomingPart.GetReactionContext();
	if (incoming.ReactionDataKey.ReactionType != EReactionType::Dead) return false;

	return InQuery.StopReason == EExecutionStopReason::Interrupted;
}

bool UCReaction_Dead::AllowInterventionBy(const FExecutionInterventionQuery& InQuery) const
{
	return false;
}
