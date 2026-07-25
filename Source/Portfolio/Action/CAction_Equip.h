#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Equip.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Equip : public UCAction
{
	GENERATED_BODY()

public:
	// Decision
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

protected:
	// Notify
	void HandleSpecificNotifyCommand(EActionNotifyCommand InCommand) override;

private:
	// Weapon
	void AttachWeapon();
};
