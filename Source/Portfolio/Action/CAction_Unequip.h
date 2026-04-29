#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Unequip.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Unequip : public UCAction
{
	GENERATED_BODY()

public:
	/* === Action Arbitration === */
	EActionExecutionDecision DecideExecution(const FActionExecutionQuery& InActionExecuteQuery) const override;

public:
	bool Start() override;

public:
	void Complete() override;
	void Abort(EActionAbortReason InActionAbortReason) override;

public:
	void DetachWeapon();
};
