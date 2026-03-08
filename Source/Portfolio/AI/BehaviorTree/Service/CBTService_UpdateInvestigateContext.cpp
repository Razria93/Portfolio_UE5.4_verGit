#include "AI/BehaviorTree/Service/CBTService_UpdateInvestigateContext.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTService_UpdateInvestigateContext::UCBTService_UpdateInvestigateContext()
{
	NodeName = TEXT("Update Investigate Context");
	bNotifyTick = true;

	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UCBTService_UpdateInvestigateContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8*, float)
{
	UWorld* world = OwnerComp.GetWorld();
	if (!IsValid(world)) return;

	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	if (!blackboardComp->GetValueAsBool(CAIKey::Investigate::bUseInvestigate)) return;
	if (!blackboardComp->GetValueAsBool(CAIKey::Investigate::bCanInvestigate)) return;
	if (!blackboardComp->GetValueAsBool(CAIKey::Investigate::bIsInvestigating)) return;

	const float lastSeenTime = blackboardComp->GetValueAsFloat(CAIKey::Perception::LastSeenTime);
	const float investigateDuration = blackboardComp->GetValueAsFloat(CAIKey::Investigate::InvestigateDuration);

	const bool bTimeout = (investigateDuration > 0.f) && ((world->GetTimeSeconds() - lastSeenTime) >= investigateDuration);

	if (bTimeout)
	{
		blackboardComp->SetValueAsBool(CAIKey::Investigate::bCanInvestigate, false);
		FLog::Log(TEXT("[Investigate Time out]"));
	}
}