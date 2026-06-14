#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Guard.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Guard : public UCAction
{
	GENERATED_BODY()

public:
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;
};
