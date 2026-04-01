#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Type/CWeaponStructure.h"
#include "CBTTask_StartAttack.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_StartAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_StartAttack();

private:
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditAnywhere, Category = "Config")
	EActionType AttackActionType = EActionType::Max;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bStopMovementOnStart = true;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
