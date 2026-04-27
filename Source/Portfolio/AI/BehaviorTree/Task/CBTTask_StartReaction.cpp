#include "AI/BehaviorTree/Task/CBTTask_StartReaction.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"

#include "Type/CWeaponStructure.h"

UCBTTask_StartReaction::UCBTTask_StartReaction()
{
	NodeName = TEXT("Start Reaction");
}

EBTNodeResult::Type UCBTTask_StartReaction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	if (!IsValid(enemy)) return EBTNodeResult::Failed;

	UCReactionComponent* reactionComp = enemy->GetReactionComp();
	if (!IsValid(reactionComp)) return EBTNodeResult::Failed;

	FReactionContext reactionContext;
	
	// Invalid pending reaction
	if (!reactionComp->TryConsumePendingReaction(reactionContext))
	{
		FLog::Log(TEXT("[StartReaction|ExecuteTask] Invalid Pending Reaction"));
	
		// If already active, keep waiting on it.
		return reactionComp->HasActiveReactionContext()
			? EBTNodeResult::Succeeded	// Go to Waiting
			: EBTNodeResult::Failed;	// Go to Root
	}
	
	if (!reactionComp->TryExecuteReaction(reactionContext))
	{
		FLog::Log(TEXT("[StartReaction|ExecuteTask] Rejected Execute reaction"));
	
		// If already active, keep waiting on it.
		return reactionComp->HasActiveReactionContext()
			? EBTNodeResult::Succeeded	// Go to Waiting
			: EBTNodeResult::Failed;	// Go to Root
	}

	FLog::Log(TEXT("[StartReaction|ExecuteTask] Succeeded Execute reaction"));
	return EBTNodeResult::Succeeded;
}