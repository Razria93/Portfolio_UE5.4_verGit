#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_Hit.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_Hit : public UCReaction
{
	GENERATED_BODY()

public:
	// Decision
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

public:
	// Observable Overlay
	void ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const override;
};
