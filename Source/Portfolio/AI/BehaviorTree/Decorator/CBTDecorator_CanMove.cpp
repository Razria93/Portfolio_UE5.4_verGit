#include "AI/BehaviorTree/Decorator/CBTDecorator_CanMove.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

#include "Component/CMovementComponent.h"

namespace
{
	TAutoConsoleVariable<int32> CVarAIRuntimeLODCanMoveDecoratorAudit(
		TEXT("Portfolio.AI.RuntimeLOD.CanMoveDecoratorAudit"),
		0,
		TEXT("Print CBTDecorator_CanMove result for runtime LOD debugging. 0: disabled, 1: enabled."),
		ECVF_Default);
}

UCBTDecorator_CanMove::UCBTDecorator_CanMove()
{
	NodeName = TEXT("Can Move");
}

bool UCBTDecorator_CanMove::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return false;

	const APawn* pawn = aiController->GetPawn();
	if (!IsValid(pawn)) return false;

	const UCMovementComponent* movementComp = pawn->FindComponentByClass<UCMovementComponent>();
	if (!IsValid(movementComp)) return false;

	const bool bCanMove = movementComp->CanAcceptMoveInput();

	if (CVarAIRuntimeLODCanMoveDecoratorAudit.GetValueOnGameThread() != 0)
	{
		FLog::Log(FString::Printf(
			TEXT("[CanMoveDecoratorAudit] Owner=%s | CanMove=%s"),
			*GetNameSafe(pawn),
			bCanMove ? TEXT("true") : TEXT("false")));
	}

	return bCanMove;
}
