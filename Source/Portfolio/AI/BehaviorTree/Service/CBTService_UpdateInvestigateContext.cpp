#include "AI/BehaviorTree/Service/CBTService_UpdateInvestigateContext.h"

#include "ProjectGlobal.h"

#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"
#include "AI/Blackboard/CAIKey.h"
#include "AI/Blackboard/CAIBlackboardValueHelper.h"

#include "ProfilingDebugging/CsvProfiler.h"
#include "BehaviorTree/BlackboardComponent.h"

UCBTService_UpdateInvestigateContext::UCBTService_UpdateInvestigateContext()
{
	NodeName = TEXT("Update Investigate Context");
	bNotifyTick = true;

	Interval = CBTServiceIntervalHelper::GetDefaultInvestigateContextInterval();
	RandomDeviation = CBTServiceIntervalHelper::GetDefaultRandomDeviation();
}

void UCBTService_UpdateInvestigateContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	CSV_SCOPED_TIMING_STAT_GLOBAL(PortfolioAI_BT_UpdateInvestigateContext);

	UWorld* world = OwnerComp.GetWorld();
	if (!IsValid(world)) return;

	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	if (!blackboardComp->GetValueAsBool(CAIKey::Investigate::bUseInvestigate.KeyName)) return;
	if (!blackboardComp->GetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName)) return;

	const float lastSeenTime = blackboardComp->GetValueAsFloat(CAIKey::Perception::LastSeenTime.KeyName);
	const float investigateDuration = blackboardComp->GetValueAsFloat(CAIKey::Investigate::InvestigateDuration.KeyName);

	const bool bTimeout = (investigateDuration > 0.f) && ((world->GetTimeSeconds() - lastSeenTime) >= investigateDuration);

	if (bTimeout)
	{
		CAIBlackboardValueHelper::SetBoolIfChanged(blackboardComp, CAIKey::Investigate::bShouldInvestigate.KeyName, false);
		CAIBlackboardValueHelper::SetBoolIfChanged(blackboardComp, CAIKey::Investigate::bShouldEndInvestigate.KeyName, true);
	}
}
