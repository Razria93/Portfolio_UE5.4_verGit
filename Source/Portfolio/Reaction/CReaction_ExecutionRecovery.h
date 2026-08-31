#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_ExecutionRecovery.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_ExecutionRecovery : public UCReaction
{
	GENERATED_BODY()

public:
	virtual FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;
	virtual bool WantIntervention(const FExecutionInterventionQuery& InQuery) const override;
};
