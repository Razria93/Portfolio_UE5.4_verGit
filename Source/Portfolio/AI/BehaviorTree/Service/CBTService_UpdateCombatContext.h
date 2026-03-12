#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Type/CAIStructure.h"
#include "CBTService_UpdateCombatContext.generated.h"

UCLASS()
class PORTFOLIO_API UCBTService_UpdateCombatContext : public UBTService
{
	GENERATED_BODY()

public:
	UCBTService_UpdateCombatContext();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	EContextBuildResult BuildCombatContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FCombatContext& OutCombatContext);

private:
	EContextBuildResult ComputeCombatContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FCombatContext& InOutCombatContext);

private:
	void UpdateCombatContext(UBlackboardComponent* InBlackboardComp, FCombatContext& InCombatContext);

private:
	void ClearCombatContext(UBlackboardComponent* InBlackboardComp);

private:
	void PrintCombatContext(const APawn* InOwnerPawn, const FCombatContext& InCombatContext, const float InCurrentTime);
};
