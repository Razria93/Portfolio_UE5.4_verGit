#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_CollapseHit.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_CollapseHit : public UCReaction
{
	GENERATED_BODY()

public:
	// Decision
	virtual FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

};
