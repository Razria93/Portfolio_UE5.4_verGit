#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_Hit.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_Hit : public UCReaction
{
	GENERATED_BODY()

public:
	EExecutionDecision ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

public:
	bool WantIntervention(const FExecutionInterventionQuery& InQuery) const override;
};
