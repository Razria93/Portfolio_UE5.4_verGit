#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_Dead.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_Dead : public UCReaction
{
	GENERATED_BODY()

public:
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;
	void ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const override;

public:
	bool AllowIntervention(const FExecutionInterventionQuery& InQuery) const override;
};
