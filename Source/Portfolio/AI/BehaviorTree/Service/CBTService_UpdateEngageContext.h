#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Type/CAIStructure.h"
#include "CBTService_UpdateEngageContext.generated.h"

UCLASS()
class PORTFOLIO_API UCBTService_UpdateEngageContext : public UBTService
{
	GENERATED_BODY()

public:
	UCBTService_UpdateEngageContext();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void ScheduleNextTick(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	EContextBuildResult BuildEngageContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FEngageContext& OutEngageContext);

private:
	EContextBuildResult ComputeEngageContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FEngageContext& InOutEngageContext);

private:
	void UpdateEngageContext(UBlackboardComponent* InBlackboardComp, FEngageContext& InEngageContext);

private:
	void ClearEngageContext(UBlackboardComponent* InBlackboardComp);
};
