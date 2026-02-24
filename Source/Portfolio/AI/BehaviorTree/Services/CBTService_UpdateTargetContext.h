#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "CBTService_UpdateTargetContext.generated.h"

UCLASS()
class PORTFOLIO_API UCBTService_UpdateTargetContext : public UBTService
{
	GENERATED_BODY()
	
public:
	UCBTService_UpdateTargetContext();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
