#include "AI/BehaviorTree/Decorator/CBTDecorator_HasCombatTargetWithLOS.h"

#include "ProjectGlobal.h"

#include "AI/Blackboard/CAIKey.h"

#include "BehaviorTree/BlackboardComponent.h"

UCBTDecorator_HasCombatTargetWithLOS::UCBTDecorator_HasCombatTargetWithLOS()
{
	NodeName = TEXT("Has Combat Target With LOS");
}

bool UCBTDecorator_HasCombatTargetWithLOS::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	if (!HasCombatTarget(OwnerComp)) return false;

	const UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	return IsValid(blackboardComp) && blackboardComp->GetValueAsBool(CAIKey::Perception::bHasLOS.KeyName);
}
