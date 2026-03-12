#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Type/CAIStructure.h"
#include "CBTService_UpdateAIContext.generated.h"

UCLASS()
class PORTFOLIO_API UCBTService_UpdateAIContext : public UBTService
{
	GENERATED_BODY()

public:
	UCBTService_UpdateAIContext();

private:
	UPROPERTY(EditAnywhere, Category = "Config")
	float MovableRange = 1000.f;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	EContextBuildResult BuildPerceptionContext(class APawn* InOwnerPawn, FAIContext& OutAIContext);

private:
	EContextBuildResult ComputeHomeMetricContext(class APawn* InOwnerPawn, class UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext);
	EContextBuildResult ComputeCombatMetricContext(class APawn* InOwnerPawn, class UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext);
	EContextBuildResult ComputeCombatAssignmentContext(class APawn* InOwnerPawn, class UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext);

private:
	void UpdatePerceptionContext(class UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext);
	void UpdateHomeMetricContext(class UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext);
	void UpdateCombatMetricContext(class UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext);
	void UpdateCombatAssignmentContext(class UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext);

private:
	void ClearPerceptionContext(UBlackboardComponent* InBlackboardComp);
	void ClearHomeMetricContext(UBlackboardComponent* InBlackboardComp);
	void ClearCombatMetricContext(UBlackboardComponent* InBlackboardComp);
	void ClearCombatAssignmentContext(UBlackboardComponent* InBlackboardComp);
};
