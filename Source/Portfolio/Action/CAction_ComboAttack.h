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
	// Decision
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

public:
	// Chain Reservation
	bool ReserveChain(const FActionData& InData) override;
	void ConsumeChain() override;

protected:
	// Lifecycle
	void ClearRuntime() override;

protected:
	// Notify
	void HandleSpecificNotifyCommand(EActionNotifyCommand InCommand) override;

public:
	// Chain Window
	void OpenReserveChainWindow();
	void CloseReserveChainWindow();

private:
	// Chain Query
	bool CanResolveChain(const FExecutionDecisionQuery& InQuery) const;	// CheckTiming: Input
	bool CanReserveChain(const FActionData& InData) const;				// CheckTiming: Reserve
	bool CanConsumeChain(const FActionData& InData) const;				// CheckTiming: Consume
};
