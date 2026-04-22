#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Equip.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Equip : public UCAction
{
	GENERATED_BODY()

public:
	bool CanStart() const override;

public:
	bool Start() override;
	void Complete() override;

public:
	void AttachWeapon();
};
