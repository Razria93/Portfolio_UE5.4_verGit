#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_ComboAttack.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_ComboAttack : public UCAction
{
	GENERATED_BODY()

private:
	int32 ActionIndex;

private:
	bool bEnablePreInput;
	bool bExistPreInput;

public:
	void InitializeAction(ACharacter* InOwnerCharacter, EActionType InActionType, const TArray<FActionData> InActionDatas) override;
	void Tick(float InDeltaTime) override;

public:
	void PlayAction() override;
	void BeginPlayAction() override;
	void EndPlayAction() override;
	void NextPlayAction() override;

public:
	FORCEINLINE void OnEnablePreInput() { bEnablePreInput = true; }
	FORCEINLINE void OffEnablePreInput() { bEnablePreInput = false; }
};
