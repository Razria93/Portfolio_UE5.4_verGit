#include "AI/BehaviorTree/Task/CBTTask_SetCombatTargetFocus.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CCombatTargetComponent.h"
#include "AI/Blackboard/CAIKey.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UCBTTask_SetCombatTargetFocus::UCBTTask_SetCombatTargetFocus()
{
	NodeName = TEXT("Set Combat Target Focus");
}

EBTNodeResult::Type UCBTTask_SetCombatTargetFocus::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(blackboardComp) || !IsValid(aiController)) return EBTNodeResult::Failed;

	const ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	const UCCombatTargetComponent* combatTargetComp = IsValid(enemy) ? enemy->GetCombatTargetComp() : nullptr;
	const FCombatTargetSnapshot snapshot = IsValid(combatTargetComp) ? combatTargetComp->GetCombatTargetSnapshot() : FCombatTargetSnapshot();
	AActor* target = snapshot.TargetActor;

	if (!IsValid(target)) return EBTNodeResult::Failed;

	if (blackboardComp->GetValueAsObject(CAIKey::CombatTarget::Actor.KeyName) != target) return EBTNodeResult::Failed;
	if (blackboardComp->GetValueAsInt(CAIKey::CombatTarget::CombatTargetRevision.KeyName) != snapshot.Revision) return EBTNodeResult::Failed;

	aiController->SetFocus(target, EAIFocusPriority::Gameplay);

	return EBTNodeResult::Succeeded;
}
