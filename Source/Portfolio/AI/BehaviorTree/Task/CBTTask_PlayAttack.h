#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_PlayAttack.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_PlayAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_PlayAttack();

private:
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<UAnimMontage*> AttackMontages;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
