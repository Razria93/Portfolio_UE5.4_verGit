#include "AI/BehaviorTree/Decorator/CBTDecorator_HasValidTarget.h"

#include "ProjectGlobal.h"

#include "AI/Blackboard/CAIKey.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CCombatTargetComponent.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UCBTDecorator_HasValidTarget::UCBTDecorator_HasValidTarget()
{
	NodeName = "Has Valid Target";
}

bool UCBTDecorator_HasValidTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return false;

	const AAIController* aiController = OwnerComp.GetAIOwner();
	const ACEnemy* enemy = IsValid(aiController) ? Cast<ACEnemy>(aiController->GetPawn()) : nullptr;
	const UCCombatTargetComponent* combatTargetComp = IsValid(enemy) ? enemy->GetCombatTargetComp() : nullptr;
	const UObject* target = IsValid(combatTargetComp) ? combatTargetComp->GetCombatTargetSnapshot().TargetActor : nullptr;
	if (!IsValid(target)) return false;

	if (bRequireLOS)
	{
		const bool bHasLOS = blackboardComp->GetValueAsBool(CAIKey::Perception::bHasLOS.KeyName);
		if (!bHasLOS) return false;
	}

	return true;
}
