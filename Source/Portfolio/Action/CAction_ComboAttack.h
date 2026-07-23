#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "Type/CActionDataTypes.h"
#include "Type/CExecutionTypes.h"
#include "CAction_ComboAttack.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_ComboAttack : public UCAction
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	bool bReserveChainWindowOpened = false;

	UPROPERTY(Transient)
	bool bHasReservingChain = false;

	UPROPERTY(Transient)
	FActionData ReservingChainData = FActionData();

public:
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

public:
	bool ReserveChain(const FActionData& InData) override;
	void ConsumeChain() override;

protected:
	void ClearRuntime() override;

protected:
	void HandleSpecificNotifyCommand(EActionNotifyCommand InCommand) override;

public:
	void OpenReserveChainWindow();
	void CloseReserveChainWindow();

private:
	bool CanResolveChain(const FExecutionDecisionQuery& InQuery) const;	// CheckTiming: Input
	bool CanReserveChain(const FActionData& InData) const;				// CheckTiming: Reserve
	bool CanConsumeChain(const FActionData& InData) const;				// CheckTiming: Consume
};
