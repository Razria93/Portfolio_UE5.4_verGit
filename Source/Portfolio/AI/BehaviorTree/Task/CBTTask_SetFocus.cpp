#include "AI/BehaviorTree/Task/CBTTask_SetFocus.h"

#include "ProjectGlobal.h"

#include "AI/Blackboard/CAIKey.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"

UCBTTask_SetFocus::UCBTTask_SetFocus()
{
	NodeName = TEXT("Set Focus");
}

EBTNodeResult::Type UCBTTask_SetFocus::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	AActor* target = Cast<AActor>(blackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName));
	if (!IsValid(target)) return EBTNodeResult::Failed;

	aiController->SetFocus(target, EAIFocusPriority::Gameplay);
	return EBTNodeResult::Succeeded;
}
