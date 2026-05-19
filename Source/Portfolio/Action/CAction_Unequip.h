#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Unequip.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Unequip : public UCAction
{
	GENERATED_BODY()

public:
	EExecutionDecision ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

protected:
	void HandleSpecificNotifyCommand(EActionNotifyCommand InCommand) override;

private:
	void DetachWeapon();
};
