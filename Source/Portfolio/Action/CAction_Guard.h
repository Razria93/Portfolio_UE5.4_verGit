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
	void Interrupt(const FExecutionInterventionDirective& InDirective) override;
	void Complete() override;

public:
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;
	bool TryResolveDeferredConsumeKey(const FExecutionDecisionQuery& InQuery, EDeferredActionConsumeKey& OutConsumeKey) const override;
	void ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const override;

public:
	bool WantIntervention(const FExecutionInterventionQuery& InQuery) const override;
	bool AllowIntervention(const FExecutionInterventionQuery& InQuery) const override;

protected:
	void HandleSpecificNotifyCommand(EActionNotifyCommand InCommand) override;

private:
	void ClearDeferredGuardActions() const;
	void ClearGuardState() const;

private:
	void PrintGuardInterventionDebugInfo(const FString& InStage, EGuardActionPhase InActiveGuardPhase, const FExecutionParticipant& InIncomingPart, bool bResult) const;
};
