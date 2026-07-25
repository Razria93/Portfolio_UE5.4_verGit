#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Dodge.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Dodge : public UCAction
{
	GENERATED_BODY()

public:
	// Decision
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

	// Observable Overlay
	void ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const override;

public:
	// Intervention
	bool WantIntervention(const FExecutionInterventionQuery& InQuery) const override;
};
