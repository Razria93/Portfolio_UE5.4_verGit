#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "CBTService_UpdateInvestigateContext.generated.h"

UCLASS()
class PORTFOLIO_API UCBTService_UpdateInvestigateContext : public UBTService
{
	GENERATED_BODY()
	
public:
	UCBTService_UpdateInvestigateContext();

protected:
	// Lifecycle
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
