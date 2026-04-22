#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Unequip.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Unequip : public UCAction
{
	GENERATED_BODY()

public:
	bool CanStart() const override;

public:
	bool Start() override;
	void Complete() override;

public:
	void DetachWeapon();
};
