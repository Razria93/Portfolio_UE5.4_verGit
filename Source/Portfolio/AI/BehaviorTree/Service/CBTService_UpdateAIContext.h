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
	float AttackRange = 200.f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float MovableRange = 1000.f;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	bool BuildPerceptionContext(class APawn* InOwnerPawn, FAIContext& OutAIContext);
	bool ComputeMetricContext(class APawn* InOwnerPawn, class UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext);

private:
	void UpdatePerceptionContext(class UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext);
	void UpdateCombatContext(class UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext);
	void UpdateNavigationContext(class UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext);
};
