#include "AI/BehaviorTree/Task/CBTTask_CommitAttackCooldown.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Character/Enemy/CEnemy.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_CommitAttackCooldown::UCBTTask_CommitAttackCooldown()
{
	NodeName = TEXT("Commit Attack Cooldown");
}

EBTNodeResult::Type UCBTTask_CommitAttackCooldown::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	if (!IsValid(enemy)) return EBTNodeResult::Failed;

	const float attackCooldown = enemy->GetAttackCooldown();
	const float currentTime = OwnerComp.GetWorld()->GetTimeSeconds();
	const float attackableTime = currentTime + attackCooldown;

	blackboardComp->SetValueAsFloat(CAIKey::Engage::AttackableTime, attackableTime);

	return EBTNodeResult::Succeeded;
}