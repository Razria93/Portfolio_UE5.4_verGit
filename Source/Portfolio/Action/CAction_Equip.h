#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Equip.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Equip : public UCAction
{
	GENERATED_BODY()

public:
	EActionLocalLevelDecision ResolveLocalLevelDecision(const FActionLocalLevelQuery& InQuery) const override;

public:
	void AttachWeapon();
};
