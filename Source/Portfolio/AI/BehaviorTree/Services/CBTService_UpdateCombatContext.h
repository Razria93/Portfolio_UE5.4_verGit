#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "CBTService_UpdateCombatContext.generated.h"

UCLASS()
class PORTFOLIO_API UCBTService_UpdateCombatContext : public UBTService
{
	GENERATED_BODY()

public:
	UCBTService_UpdateCombatContext();

private:
	UPROPERTY(EditAnywhere, Category = "Config")
	float AttackRange = 200.f;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
