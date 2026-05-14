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
	bool bChainWindowOpened = false;

	UPROPERTY(Transient)
	bool bHasChainedInput = false;

	UPROPERTY(Transient)
	FActionData PendingChainData_Cached = FActionData();

public:
	void InitializeAction(ACharacter* InOwnerCharacter, class UCActionComponent* InOwnerActionComp) override;

public:
	EActionLocalLevelDecision ResolveLocalLevelDecision(const FActionLocalLevelQuery& InQuery) const override;

public:
	bool Start(const FActionData& InData) override;
	bool ApplyChain(const FActionData& InData) override;
	void Stop(EActionStopReason InStopReason) override;
	void Complete() override;

public:
	void AdvanceCombo();

public:
	void OpenChainWindow();
	void CloseChainWindow();

private:
	void ClearComboRuntime();

private:
	bool CanAcceptChain(const FActionData& InData) const;
	bool CanAdvanceCombo() const;
};
