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
	void InitializeAction(ACharacter* InOwnerCharacter, EActionType InActionType, const TArray<FActionData>& InActionDatas) override;

public:
	/* === Action Arbitration === */
	EActionExecutionDecision DecideExecution(const FActionExecutionQuery& InActionExecuteQuery) const override;

public:
	bool Start() override;
	bool ApplyChain(const FActionExecutionQuery& InActionExecuteQuery) override;

public:
	void Complete() override;
	void Abort(EActionAbortReason InActionAbortReason) override;

public:
	void OpenComboPreInput();
	void CloseComboPreInput();

public:
	void AdvanceCombo();

protected:
	FActionContext BuildActionContext() const override;
	FActionFeedbackRequest BuildFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const override;

public:
	FORCEINLINE void EnablePreInput() { bEnablePreInput = true; }
	FORCEINLINE void DisablePreInput() { bEnablePreInput = false; }
};
