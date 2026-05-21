#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Dodge.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Dodge : public UCAction
{
	GENERATED_BODY()

public:
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

public:
	bool MatchesWantIntervention(const FExecutionInterventionQuery& InQuery) const override;
};
