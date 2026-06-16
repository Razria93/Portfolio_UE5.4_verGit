#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_Hit.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_Hit : public UCReaction
{
	GENERATED_BODY()

public:
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;
	void ResolveObservableOverlayExecutionCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const override;
};
