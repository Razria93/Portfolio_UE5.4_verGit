#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifyQueue.h"

class UCActionComponent;
class UCReactionComponent;

// New defensive executions require the originating montage instance, not just an asset/type match.
namespace CCombatNotifyOwnership
{
	PORTFOLIO_API bool MatchesMontageInstance(const FAnimNotifyEventReference& InReference, int32 InExpectedInstanceId);
	PORTFOLIO_API bool AllowsAction(const UCActionComponent* InComponent, const FAnimNotifyEventReference& InReference);
	PORTFOLIO_API bool AllowsReaction(const UCReactionComponent* InComponent, const FAnimNotifyEventReference& InReference);
}
