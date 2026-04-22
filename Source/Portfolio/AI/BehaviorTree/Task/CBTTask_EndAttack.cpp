#include "AI/BehaviorTree/Task/CBTTask_EndAttack.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"

#include "Type/CWeaponStructure.h"
#include "AI/BlackBoard/CAIKey.h"

UCBTTask_EndAttack::UCBTTask_EndAttack()
{
	NodeName = TEXT("End Attack");
}

EBTNodeResult::Type UCBTTask_EndAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACharacter* character = Cast<ACharacter>(aiController->GetPawn());
	if (!IsValid(character)) return EBTNodeResult::Failed;

	if (IsValid(blackboardComp))
	{
		blackboardComp->SetValueAsInt(CAIKey::Engage::AttackIndex, INDEX_NONE);
		blackboardComp->SetValueAsEnum(CAIKey::Engage::AttackActionType, static_cast<uint8>(EActionType::Max));

		FLog::Log(TEXT("[EndAttack] Cleared current attack context"));
	}

	if (UCWeaponComponent* weaponComp = character->FindComponentByClass<UCWeaponComponent>())
	{
		weaponComp->ClearContext();
	}

	if (ACEnemy* enemy = Cast<ACEnemy>(character))
	{
		enemy->ClearActiveActionFeedbackKey();
	}

	UCMovementComponent* movementComp = character->FindComponentByClass<UCMovementComponent>();
	if (IsValid(movementComp))
	{
		movementComp->SetMove();
	}

	return EBTNodeResult::Succeeded;
}
