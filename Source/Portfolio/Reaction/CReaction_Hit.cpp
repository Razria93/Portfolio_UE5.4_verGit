#include "Reaction/CReaction_Hit.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

EExecutionDecision UCReaction_Hit::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	if (!InQuery.IncomingPart.IsReactionParticipant()) return EExecutionDecision::Reject;
	if (InQuery.Snapshot.IsDead()) return EExecutionDecision::Reject;

	const FReactionExecutionContext& incoming = InQuery.IncomingPart.GetReactionContext();
	if (incoming.ReactionDataKey.ReactionType != EReactionType::Hit) return EExecutionDecision::Reject;

	return EExecutionDecision::Executable;
}

bool UCReaction_Hit::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!InQuery.IncomingPart.IsReactionParticipant()) return false;

	const FReactionExecutionContext& incoming = InQuery.IncomingPart.GetReactionContext();
	if (incoming.ReactionDataKey.ReactionType != EReactionType::Hit) return false;

	return InQuery.StopReason == EExecutionStopReason::Interrupted;
}
