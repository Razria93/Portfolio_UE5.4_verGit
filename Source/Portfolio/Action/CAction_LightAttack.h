#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_LightAttack.generated.h"

UCLASS(Blueprintable)
class PORTFOLIO_API UCAction_LightAttack : public UCAction
{
	GENERATED_BODY()

public:
	void Tick(float InDeltaTime) override;

public:
	void PlayAction() override;
	void BeginPlayAction() override;
	void EndPlayAction() override;
};