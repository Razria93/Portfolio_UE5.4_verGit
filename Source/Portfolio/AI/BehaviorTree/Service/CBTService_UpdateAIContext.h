#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Type/CAITypes.h"
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
	virtual void ScheduleNextTick(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	EContextBuildResult BuildPerceptionContext(class APawn* InOwnerPawn, FAIBlackboardUpdateContext& OutAIContext);

private:
	EContextBuildResult ComputeHomeMetricContext(class APawn* InOwnerPawn, class UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext);
	EContextBuildResult ComputeAlertRangeContext(class APawn* InOwnerPawn, class UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext);
	EContextBuildResult ComputeEngageAssignmentContext(class APawn* InOwnerPawn, class UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext);
	EContextBuildResult ComputeReactionContext(class APawn* InOwnerPawn, class UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext);
	EContextBuildResult ComputeDeadContext(class APawn* InOwnerPawn, class UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext);

private:
	void UpdatePerceptionContext(class UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext);
	void UpdateHomeMetricContext(class UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext);
	void UpdateAlertRangeContext(class UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext);
	void UpdateEngageAssignmentContext(class UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext);
	void UpdateReactionContext(class UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext);
	void UpdateDeadContext(class UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext);

private:
	void ClearPerceptionContext(class UBlackboardComponent* InBlackboardComp);
	void ClearHomeMetricContext(class UBlackboardComponent* InBlackboardComp);
	void ClearAlertRangeContext(class UBlackboardComponent* InBlackboardComp);
	void ClearEngageAssignmentContext(class UBlackboardComponent* InBlackboardComp);
	void ClearReactionContext(class UBlackboardComponent* InBlackboardComp);
	void ClearDeadContext(class UBlackboardComponent* InBlackboardComp);
};
