#pragma once

#include "CoreMinimal.h"
#include "Action/CAction.h"
#include "CAction_Execution.generated.h"

UCLASS()
class PORTFOLIO_API UCAction_Execution : public UCAction
{
	GENERATED_BODY()

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences) override;

	// Execution Arbitration
	FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const override;

protected:
	// Execution Notify
	void HandleSpecificNotifyCommand(EActionNotifyCommand InCommand) override;

private:
	UPROPERTY(Transient)
	class UCExecutionCollaborationComponent* ExecutionCollaborationComp_Injected = nullptr;
};
