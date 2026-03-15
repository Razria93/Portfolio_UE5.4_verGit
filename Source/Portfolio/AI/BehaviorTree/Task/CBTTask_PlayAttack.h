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
	UAnimMontage* AttackMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FName> AttackSections;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	bool ResolveAttackSection(UBlackboardComponent* InBlackboardComp, FName& OutSectionName) const;
};
