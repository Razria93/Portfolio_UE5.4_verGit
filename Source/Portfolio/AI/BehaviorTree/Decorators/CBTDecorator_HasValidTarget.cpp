#include "AI/BehaviorTree/Decorators/CBTDecorator_HasValidTarget.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTDecorator_HasValidTarget::UCBTDecorator_HasValidTarget()
{
	NodeName = "Has ValidTarget";
}

bool UCBTDecorator_HasValidTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return false;

	const UObject* target = blackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor);
	if (!IsValid(target)) return false;

	if (bRequireLOS)
	{
		const bool bHasLOS = blackboardComp->GetValueAsBool(CAIKey::Perception::bHasLOS);
		if (!bHasLOS) return false;
	}

	return true;
}
