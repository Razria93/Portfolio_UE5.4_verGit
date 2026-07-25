#pragma once

#include "CoreMinimal.h"
#include "Reaction/CReaction.h"
#include "CReaction_Parry.generated.h"

UCLASS()
class PORTFOLIO_API UCReaction_Parry : public UCReaction
{
	GENERATED_BODY()

public:
	// Decision
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

public:
	// Intervention
	bool WantIntervention(const FExecutionInterventionQuery& InQuery) const override;

	// Observable Overlay
	void ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const override;
};
