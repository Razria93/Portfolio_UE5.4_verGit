#include "AI/BehaviorTree/Decorator/CBTDecorator_HasCombatTargetWithLOS.h"

#include "ProjectGlobal.h"

#include "AI/Blackboard/CAIKey.h"
#include "Controller/CAIController.h"

#include "BehaviorTree/BlackboardComponent.h"

UCBTDecorator_HasCombatTargetWithLOS::UCBTDecorator_HasCombatTargetWithLOS()
{
	NodeName = TEXT("Has Combat Target With LOS");
}

bool UCBTDecorator_HasCombatTargetWithLOS::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	if (!HasCombatTarget(OwnerComp)) return false;

	const UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	const ACAIController* controller = Cast<ACAIController>(OwnerComp.GetAIOwner());
	const AActor* target = blackboardComp ? Cast<AActor>(blackboardComp->GetValueAsObject(CAIKey::CombatTarget::Actor.KeyName)) : nullptr;
	return controller && controller->HasCurrentSightOf(target);
}
