#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Guard.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Guard : public UCAction
{
	GENERATED_BODY()

public:
	bool Start(const FActionData& InData) override;
	void Stop(EActionStopReason InStopReason) override;
	void Complete() override;

protected:
	void HandleSpecificNotifyCommand(EActionNotifyCommand InCommand) override;

public:
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;
	bool TryResolveDeferredConsumeKey(const FExecutionDecisionQuery& InQuery, EDeferredActionConsumeKey& OutConsumeKey) const override;
	void ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const override;

public:
	bool WantIntervention(const FExecutionInterventionQuery& InQuery) const override;
	bool AllowIntervention(const FExecutionInterventionQuery& InQuery) const override;
};
