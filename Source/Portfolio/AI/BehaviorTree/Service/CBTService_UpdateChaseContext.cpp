#include "AI/BehaviorTree/Service/CBTService_UpdateChaseContext.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTService_UpdateChaseContext::UCBTService_UpdateChaseContext()
{
	NodeName = TEXT("Update Chase Context");
	bNotifyTick = true;

	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UCBTService_UpdateChaseContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	const float dist = blackboardComp->GetValueAsFloat(CAIKey::Metric::DistanceToTarget);
	const float chaseOffsetDist = blackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseOffsetDintance);
	const float chaseEnterBuffer = blackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseEnterBuffer);
	const float chaseExitBuffer = blackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseExitBuffer);

	bool bCanChase = blackboardComp->GetValueAsBool(CAIKey::Chase::bCanChase);

	const float chaseEnterDist = chaseOffsetDist + chaseEnterBuffer;
	const float chaseExitDist = FMath::Max(0.f, chaseOffsetDist - chaseExitBuffer);

	if (!bCanChase)
	{
		if (dist > chaseEnterBuffer) blackboardComp->SetValueAsBool(CAIKey::Chase::bCanChase, true);
	}
	else
	{
		if (dist <= chaseExitDist) blackboardComp->SetValueAsBool(CAIKey::Chase::bCanChase, false);
	}
}
