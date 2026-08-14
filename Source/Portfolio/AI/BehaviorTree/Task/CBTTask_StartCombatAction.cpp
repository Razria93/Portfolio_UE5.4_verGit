#include "AI/BehaviorTree/Task/CBTTask_StartCombatAction.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CCombatTargetComponent.h"
#include "AI/Blackboard/CAIKey.h"
#include "Core/Debug/FAICombatBTDebug.h"
#include "Type/CActionOrchestrationTypes.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UCBTTask_StartCombatAction::UCBTTask_StartCombatAction()
{
	NodeName = TEXT("Start Combat Action");
}

EBTNodeResult::Type UCBTTask_StartCombatAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp))
	{
		FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit(nullptr, nullptr, nullptr, CombatActionIntent, FActionRequestResult(), TEXT("MissingBlackboard"));
		return EBTNodeResult::Failed;
	}

	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController))
	{
		FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit(aiController, nullptr, nullptr, CombatActionIntent, FActionRequestResult(), TEXT("MissingAIController"));
		return EBTNodeResult::Failed;
	}

	ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	const UCCombatTargetComponent* combatTargetComp = IsValid(enemy) ? enemy->GetCombatTargetComp() : nullptr;
	const FCombatTargetSnapshot targetSnapshot = IsValid(combatTargetComp) ? combatTargetComp->GetCombatTargetSnapshot() : FCombatTargetSnapshot();
	AActor* targetActor = targetSnapshot.TargetActor;
	if (!IsValid(enemy))
	{
		FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit(aiController, aiController->GetPawn(), targetActor, CombatActionIntent, FActionRequestResult(), TEXT("InvalidEnemyPawn"));
		return EBTNodeResult::Failed;
	}
	if (!IsValid(targetActor))
	{
		FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit(aiController, enemy, targetActor, CombatActionIntent, FActionRequestResult(), TEXT("MissingCombatTarget"));
		return EBTNodeResult::Failed;
	}
	if (blackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName) != targetActor
		|| blackboardComp->GetValueAsInt(CAIKey::Targeting::CombatTargetRevision.KeyName) != targetSnapshot.Revision)
	{
		FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit(aiController, enemy, targetActor, CombatActionIntent, FActionRequestResult(), TEXT("StaleCombatTargetProjection"));
		return EBTNodeResult::Failed;
	}

	const bool bCanCombatAction = blackboardComp->GetValueAsBool(CAIKey::Engage::bCanCombatAction.KeyName);
	if (!bCanCombatAction)
	{
		FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit(aiController, enemy, targetActor, CombatActionIntent, FActionRequestResult(), TEXT("CannotCombatAction"));
		return EBTNodeResult::Failed;
	}

	const bool bIsCombatAction = blackboardComp->GetValueAsBool(CAIKey::Engage::bIsCombatAction.KeyName);
	if (bIsCombatAction)
	{
		FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit(aiController, enemy, targetActor, CombatActionIntent, FActionRequestResult(), TEXT("AlreadyCombatAction"));
		return EBTNodeResult::Failed;
	}

	if (bStopMovementOnStart)
	{
		aiController->StopMovement();
	}

	const FActionRequestResult requestResult = enemy->HandleAICombatAction(CombatActionIntent, targetSnapshot);
	if (!requestResult.IsStartedResult())
	{
		FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit(aiController, enemy, targetActor, CombatActionIntent, requestResult, TEXT("ActionRequestFailed"));
		return EBTNodeResult::Failed;
	}

	const float currentTime = OwnerComp.GetWorld()->GetTimeSeconds();
	const float cooldown = enemy->GetCombatActionCooldown();
	const float nextCombatActionTime = currentTime + cooldown;
	
	blackboardComp->SetValueAsFloat(CAIKey::Engage::NextCombatActionTime.KeyName, nextCombatActionTime);
	FAICombatBTDebug::RecordCombatActionTaskSucceededForAudit(aiController, enemy, targetActor, CombatActionIntent, requestResult, cooldown, nextCombatActionTime);

	return EBTNodeResult::Succeeded;
}
