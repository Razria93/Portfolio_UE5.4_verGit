#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "CBTService_UpdateMetrics.generated.h"

/**
 * Sync target-related data into Blackboard.
 */
UCLASS()
class PORTFOLIO_API UCBTService_UpdateMetrics : public UBTService
{
	GENERATED_BODY()

public:
	UCBTService_UpdateMetrics();

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard|Read")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard|Write")
	FBlackboardKeySelector DistanceToTargetKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard|Write")
	FBlackboardKeySelector IsInCombatKey;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
