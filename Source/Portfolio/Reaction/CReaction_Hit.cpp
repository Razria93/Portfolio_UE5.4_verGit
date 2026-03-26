#include "Reaction/CReaction_Hit.h"
#include "ProjectGlobal.h"

bool UCReaction_Hit::WantToInterrupt(const FReactionQueryContext& InContext) const
{
	// Hit reaction can interrupt others
	return true;
}

bool UCReaction_Hit::WantToCancel(const FReactionQueryContext& InContext) const
{
	// Hit reaction cannot be canceled by policy
	return false;
}

bool UCReaction_Hit::AllowInterruptionBy(const FReactionQueryContext& InContext) const
{
	// Follow current interruptible window
	return IsInterruptibleNow();
}

bool UCReaction_Hit::AllowCancelBy(const FReactionQueryContext& InContext) const
{
	// Follow current cancelable window
	return IsCancelableNow();
}