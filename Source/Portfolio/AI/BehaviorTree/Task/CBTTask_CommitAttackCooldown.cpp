#include "AI/BehaviorTree/Task/CBTTask_CommitAttackCooldown.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_CommitAttackCooldown::UCBTTask_CommitAttackCooldown()
{
	NodeName = TEXT("Commit Attack Cooldown");
}

EBTNodeResult::Type UCBTTask_CommitAttackCooldown::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	const float attackCooldown = blackboardComp->GetValueAsFloat(CAIKey::Combat::AttackCooldown);
	const float currentTime = OwnerComp.GetWorld()->GetTimeSeconds();
	const float attackableTime = currentTime + attackCooldown;

	blackboardComp->SetValueAsFloat(CAIKey::Combat::AttackableTime, attackableTime);
	blackboardComp->SetValueAsBool(CAIKey::Combat::bCanAttack, false);

	return EBTNodeResult::Succeeded;
}