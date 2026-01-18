#include "Reaction/CReaction_Dead.h"
#include "ProjectGlobal.h"

bool UCReaction_Dead::WantToInterrupt(const FReactionQueryContext& InContext) const
{
	// Dead reaction can interrupt others
	return true;
}

bool UCReaction_Dead::WantToCancel(const FReactionQueryContext& InContext) const
{
	// Dead reaction cannot be canceled by policy
	return false;
}

bool UCReaction_Dead::AllowInterruptionBy(const FReactionQueryContext& InContext) const
{
	// Dead reaction cannot be interrupted
	return false;
}

bool UCReaction_Dead::AllowCancelBy(const FReactionQueryContext& InContext) const
{
	// Dead reaction cannot be canceled
	return false;
}