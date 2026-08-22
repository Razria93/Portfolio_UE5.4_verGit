#include "AI/BehaviorTree/Decorator/CBTDecorator_HasCombatTarget.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CCombatTargetComponent.h"

#include "AIController.h"

UCBTDecorator_HasCombatTarget::UCBTDecorator_HasCombatTarget()
{
	NodeName = TEXT("Has Combat Target");
}

bool UCBTDecorator_HasCombatTarget::HasCombatTarget(const UBehaviorTreeComponent& InOwnerComp) const
{
	const AAIController* aiController = InOwnerComp.GetAIOwner();
	const ACEnemy* enemy = IsValid(aiController) ? Cast<ACEnemy>(aiController->GetPawn()) : nullptr;
	const UCCombatTargetComponent* combatTargetComp = IsValid(enemy) ? enemy->GetCombatTargetComp() : nullptr;
	return IsValid(combatTargetComp) && IsValid(combatTargetComp->GetCombatTargetSnapshot().TargetActor);
}

bool UCBTDecorator_HasCombatTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return HasCombatTarget(OwnerComp);
}
