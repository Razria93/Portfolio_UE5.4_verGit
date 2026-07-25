#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_Dead.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_Dead : public UCReaction
{
	GENERATED_BODY()

public:
	// Decision
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

public:
	// Observable Overlay
	void ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const override;

public:
	// Intervention
	bool AllowIntervention(const FExecutionInterventionQuery& InQuery) const override;
};
