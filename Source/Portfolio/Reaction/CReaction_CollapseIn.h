#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_CollapseIn.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_CollapseIn : public UCReaction
{
	GENERATED_BODY()

public:
	virtual FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;
	virtual bool WantIntervention(const FExecutionInterventionQuery& InQuery) const override;
};
