#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Guard.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Guard : public UCAction
{
	GENERATED_BODY()

public:
	// Lifecycle
	bool Start(const FActionData& InData) override;
	void Interrupt(const FExecutionInterventionDirective& InDirective) override;
	void Complete() override;

public:
	// Decision
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;
	bool TryResolveDeferredConsumeKey(const FExecutionDecisionQuery& InQuery, EDeferredActionConsumeKey& OutConsumeKey) const override;

	// Observable Overlay
	void ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const override;

protected:
	// Notify
	void HandleSpecificNotifyCommand(EActionNotifyCommand InCommand) override;

public:
	// Intervention
	bool WantIntervention(const FExecutionInterventionQuery& InQuery) const override;
	bool AllowIntervention(const FExecutionInterventionQuery& InQuery) const override;

private:
	// Guard State Cleanup
	void ClearDeferredGuardActions() const;
	void ClearGuardState() const;
};
