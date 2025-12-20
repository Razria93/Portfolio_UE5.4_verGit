#pragma once

#include "CoreMinimal.h"
#include "Weapon/CAction.h"
#include "CAction_LightAttack.generated.h"

UCLASS(Blueprintable)
class PORTFOLIO_API UCAction_LightAttack : public UCAction
{
	GENERATED_BODY()

public:
	void Tick(float InDeltaTime) override;

public:
	void PlayAction() override;
	void Begin_PlayAction() override;
	void End_PlayAction() override;
};