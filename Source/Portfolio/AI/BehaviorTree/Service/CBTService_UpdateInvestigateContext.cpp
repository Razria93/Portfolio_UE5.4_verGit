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

void UCBTService_UpdateInvestigateContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UWorld* world = OwnerComp.GetWorld();
	if (!IsValid(world)) return;

	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	if (!blackboardComp->GetValueAsBool(CAIKey::Investigate::bUseInvestigate.KeyName)) return;
	if (!blackboardComp->GetValueAsBool(CAIKey::Investigate::bCanInvestigate.KeyName)) return;
	if (!blackboardComp->GetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName)) return;

	const float lastSeenTime = blackboardComp->GetValueAsFloat(CAIKey::Perception::LastSeenTime.KeyName);
	const float investigateDuration = blackboardComp->GetValueAsFloat(CAIKey::Investigate::InvestigateDuration.KeyName);

	const bool bTimeout = (investigateDuration > 0.f) && ((world->GetTimeSeconds() - lastSeenTime) >= investigateDuration);

	if (bTimeout)
	{
		blackboardComp->SetValueAsBool(CAIKey::Investigate::bCanInvestigate.KeyName, false);
		FLog::Log(TEXT("[Investigate Time out]"));
	}
}