#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Equip.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Equip : public UCAction
{
	GENERATED_BODY()

public:
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

protected:
	void HandleSpecificNotifyCommand(EActionNotifyCommand InCommand) override;

private:
	void AttachWeapon();
};
