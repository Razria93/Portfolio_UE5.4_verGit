#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_Parry.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_Parry : public UCReaction
{
	GENERATED_BODY()

public:
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

public:
	void ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const override;
};
