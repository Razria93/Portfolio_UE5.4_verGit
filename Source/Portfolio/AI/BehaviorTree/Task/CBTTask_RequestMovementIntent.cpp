#include "AI/BehaviorTree/Task/CBTTask_RequestMovementIntent.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Type/CActionOrchestrationTypes.h"

#include "AIController.h"

UCBTTask_RequestMovementIntent::UCBTTask_RequestMovementIntent()
{
	NodeName = TEXT("Request Movement Intent");
}

EBTNodeResult::Type UCBTTask_RequestMovementIntent::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	if (!IsValid(enemy)) return EBTNodeResult::Failed;

	FActionRequestResult requestResult;

	switch (MovementIntent)
	{
	case EMovementActionIntent::Walk:		requestResult = enemy->HandleAIWalk(); break;
	case EMovementActionIntent::Run:		requestResult = enemy->HandleAIRun(); break;
	case EMovementActionIntent::Sprint:		requestResult = enemy->HandleAISprint(); break;
	case EMovementActionIntent::Jump:		requestResult = enemy->HandleAIJump(); break;
	case EMovementActionIntent::StopJump:	requestResult = enemy->HandleAIStopJump(); break;
	default:
		return EBTNodeResult::Failed;
	}

	return requestResult.IsAccepted() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
