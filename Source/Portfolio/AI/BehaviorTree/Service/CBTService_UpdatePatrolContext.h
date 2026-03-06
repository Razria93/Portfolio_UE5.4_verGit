#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Type/CAIStructure.h"
#include "CBTService_UpdatePatrolContext.generated.h"

UCLASS()
class PORTFOLIO_API UCBTService_UpdatePatrolContext : public UBTService
{
	GENERATED_BODY()
	
public:
	UCBTService_UpdatePatrolContext();

private:
	UPROPERTY(EditAnywhere, Category = "Config")
	float ReachThreshold_XY = 100.f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float Tolerance_Z = 200.f;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	EContextBuildResult BuildPatrolContext(class APawn* InOwnerPawn, class UBlackboardComponent* InBlackboardComp, FPatrolContext& OutPatrolContext);

private:
	EContextBuildResult ComputePatrolContext(class APawn* InOwnerPawn, class UBlackboardComponent* InBlackboardComp, FPatrolContext& InOutPatrolContext);
	bool ComputeNextIndex(int32 InCount, bool& InOutReverse, EPatrolMode InPatrolMode, int32 InCurrentIndex, int32& OutNextIndex);

private:
	void UpdatePatrolContext(class UBlackboardComponent* InBlackboardComp, const FPatrolContext& InPatrolContext);
	void ClearPatrolContext(class UBlackboardComponent* InBlackboardComp);

private:
	bool IsReached(const FVector& InOwnerLocation, const FVector& InPatrolLocation) const;

private:
	void PrintPatrolContextData(const APawn* InOwnerPawn, const UBlackboardComponent* InBlackboardComp);
};


