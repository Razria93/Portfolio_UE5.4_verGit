#include "AI/BehaviorTree/Decorator/CBTDecorator_CanMove.h"

#include "ProjectGlobal.h"

#include "Core/Debug/FAICombatBTDebug.h"
#include "Component/CMovementComponent.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"

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
	FAICombatBTDebug::RecordCanMoveDecoratorResultForAudit(pawn, bCanMove);

	return bCanMove;
}
