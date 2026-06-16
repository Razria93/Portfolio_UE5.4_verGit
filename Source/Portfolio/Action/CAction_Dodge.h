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
	void ResolveObservableOverlayExecutionCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const override;

public:
	bool WantIntervention(const FExecutionInterventionQuery& InQuery) const override;
};
