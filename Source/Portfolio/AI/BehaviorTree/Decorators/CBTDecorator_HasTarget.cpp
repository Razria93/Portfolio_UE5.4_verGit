#include "AI/BehaviorTree/Decorators/CBTDecorator_HasTarget.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTDecorator_HasTarget::UCBTDecorator_HasTarget()
{
	NodeName = "Has Target";
	TargetActorKey.SelectedKeyName = CAIKey::TargetActor;
}

bool UCBTDecorator_HasTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* blackboard = OwnerComp.GetBlackboardComponent();
	if (!blackboard) return false;

	return blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName) != nullptr;
}
