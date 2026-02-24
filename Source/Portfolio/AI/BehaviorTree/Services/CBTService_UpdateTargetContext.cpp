#include "AI/BehaviorTree/Services/CBTService_UpdateTargetContext.h"
#include "ProjectGlobal.h"

#include "Controller/CAIController.h"

UCBTService_UpdateTargetContext::UCBTService_UpdateTargetContext()
{
	NodeName = "Update TargetContext";

	bNotifyTick = true;

	Interval = 0.2f;
	RandomDeviation = 0.05f;
}

void UCBTService_UpdateTargetContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* temp = OwnerComp.GetAIOwner();
	if (!temp) return;

	ACAIController* aiController = Cast<ACAIController>(temp);
	if (!aiController) return;

	aiController->UpdateTargetDataMap();
	aiController->UpdateBlackboardContext();
}
