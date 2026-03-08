#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "CBTService_UpdateChaseContext.generated.h"

UCLASS()
class PORTFOLIO_API UCBTService_UpdateChaseContext : public UBTService
{
	GENERATED_BODY()
	
public:
	UCBTService_UpdateChaseContext();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
