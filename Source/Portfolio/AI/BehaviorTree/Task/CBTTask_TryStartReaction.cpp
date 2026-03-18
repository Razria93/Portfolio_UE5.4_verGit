#include "AI/BehaviorTree/Task/CBTTask_TryStartReaction.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CReactionComponent.h"

#include "Type/CWeaponStructure.h"

UCBTTask_TryStartReaction::UCBTTask_TryStartReaction()
{
	NodeName = TEXT("Try Start Reaction");
}

EBTNodeResult::Type UCBTTask_TryStartReaction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	if (!IsValid(enemy)) return EBTNodeResult::Failed;

	UCReactionComponent* reactionComp = enemy->GetReactionComponent();
	if (!IsValid(reactionComp)) return EBTNodeResult::Failed;

	FReactionContext reactionContext;

	// No pending reaction
	if (!reactionComp->TryConsumePendingReaction(reactionContext))
	{
		// If already active, keep waiting on it.
		return reactionComp->HasActiveReactionContext()
			? EBTNodeResult::Succeeded	// Go to Waiting
			: EBTNodeResult::Failed;	// Go to Root
	}

	// Execution be rejected
	if (!reactionComp->TryExecuteReaction(reactionContext))
	{
		// If already active, keep waiting on it.
		return reactionComp->HasActiveReactionContext()
			? EBTNodeResult::Succeeded	// Go to Waiting
			: EBTNodeResult::Failed;	// Go to Root
	}

	return EBTNodeResult::Succeeded;
}