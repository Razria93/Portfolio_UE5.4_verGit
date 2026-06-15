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

public:
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;
};
