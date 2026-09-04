#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Execution.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Execution : public UCAction
{
	GENERATED_BODY()

public:
	// Execution Arbitration
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

protected:
	// Execution Notify Bridge
	void HandleSpecificNotifyCommand(EActionNotifyCommand InCommand) override;
};
