#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_Dead.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_Dead : public UCReaction
{
	GENERATED_BODY()

public:
	bool WantToInterrupt(const FReactionQueryContext& InReactionQueryContext) const override;
	bool WantToCancel(const FReactionQueryContext& InReactionQueryContext) const override;
	bool AllowInterruptionBy(const FReactionQueryContext& InReactionQueryContext) const override;
	bool AllowCancelBy(const FReactionQueryContext& InReactionQueryContext) const override;
};