#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "Type/CWeaponStructure.h"
#include "CAction_ComboAttack.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_ComboAttack : public UCAction
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	int32 ActionIndex;

private:
	UPROPERTY(Transient)
	bool bEnablePreInput;

	UPROPERTY(Transient)
	bool bExistPreInput;

public:
	void InitializeAction(ACharacter* InOwnerCharacter, EActionType InActionType, const TArray<FActionData> InActionDatas) override;
	void Tick(float InDeltaTime) override;

public:
	bool PlayAction() override;
	void BeginPlayAction() override;
	void EndPlayAction() override;
	void NextPlayAction() override;

protected:
	FActionContext BuildActionContext() const override;
	FActionFeedbackRequest BuildActionFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const override;

public:
	FORCEINLINE void OnEnablePreInput() { bEnablePreInput = true; }
	FORCEINLINE void OffEnablePreInput() { bEnablePreInput = false; }
};
