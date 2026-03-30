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
	
	// Invalid pending reaction
	if (!reactionComp->TryConsumePendingReaction(reactionContext))
	{
		FLog::Log(TEXT("[TryStartReaction|ExecuteTask] Invalid Pending Reaction"));
	
		// If already active, keep waiting on it.
		return reactionComp->HasActiveReactionContext()
			? EBTNodeResult::Succeeded	// Go to Waiting
			: EBTNodeResult::Failed;	// Go to Root
	}
	
	// Reject Execute reaction
	if (!reactionComp->TryExecuteReaction(reactionContext))
	{
		FLog::Log(TEXT("[TryStartReaction|ExecuteTask] Rejected Execute reaction"));
	
		// If already active, keep waiting on it.
		return reactionComp->HasActiveReactionContext()
			? EBTNodeResult::Succeeded	// Go to Waiting
			: EBTNodeResult::Failed;	// Go to Root
	}

	FLog::Log(TEXT("[TryStartReaction|ExecuteTask] Succeeded Execute reaction"));
	return EBTNodeResult::Succeeded;
}