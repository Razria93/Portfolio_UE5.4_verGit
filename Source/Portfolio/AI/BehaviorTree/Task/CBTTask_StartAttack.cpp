#include "AI/BehaviorTree/Task/CBTTask_StartAttack.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_StartAttack::UCBTTask_StartAttack()
{
	NodeName = TEXT("Start Attack");
}

EBTNodeResult::Type UCBTTask_StartAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	// Early Return
	const bool bIsAttacking = blackboardComp->GetValueAsBool(CAIKey::Engage::bIsAttacking);
	if (bIsAttacking) return EBTNodeResult::Succeeded;

	const bool bCanAttack = blackboardComp->GetValueAsBool(CAIKey::Engage::bCanAttack);
	if (!bCanAttack) return EBTNodeResult::Failed;

	const int32 attackIndex = blackboardComp->GetValueAsInt(CAIKey::Engage::AttackIndex);
	if (!AttackMontages.IsValidIndex(attackIndex)) return EBTNodeResult::Failed;

	if (AttackActionType == EActionType::Max) return EBTNodeResult::Failed;

	UAnimMontage* attackMontage = AttackMontages[attackIndex];
	if (!IsValid(attackMontage)) return EBTNodeResult::Failed;

	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACharacter* character = Cast<ACharacter>(aiController->GetPawn());
	if (!IsValid(character)) return EBTNodeResult::Failed;

	USkeletalMeshComponent* meshComp = character->GetMesh();
	if (!IsValid(meshComp)) return EBTNodeResult::Failed;

	UAnimInstance* animInstance = meshComp->GetAnimInstance();
	if (!IsValid(animInstance)) return EBTNodeResult::Failed;

	if (bStopMovementOnStart)
	{
		aiController->StopMovement();

		if (UCharacterMovementComponent* movementComp = character->GetCharacterMovement())
		{
			movementComp->StopMovementImmediately();
		}
	}

	// Play Attack
	const float duration = animInstance->Montage_Play(attackMontage);
	if (duration <= 0.f) return EBTNodeResult::Failed;

	if (UCWeaponComponent* weaponComp = character->FindComponentByClass<UCWeaponComponent>())
	{
		FActionContext actionContext;
		actionContext.CurrentActionType = AttackActionType;
		actionContext.ActionIndex = attackIndex;

		weaponComp->PushContextToAttachment(actionContext);
	}

	if (ACEnemy* enemy = Cast<ACEnemy>(character))
	{
		enemy->CacheActiveActionFeedbackKey(AttackActionType, attackIndex);
	}

	if (bStopMovementOnStart)
	{
		if (UCMovementComponent* movementComp = character->FindComponentByClass<UCMovementComponent>())
		{
			movementComp->SetStop();
		}
	}

	blackboardComp->SetValueAsInt(CAIKey::Engage::LastAttackIndex, attackIndex);
	blackboardComp->SetValueAsInt(CAIKey::Engage::AttackIndex, attackIndex);
	blackboardComp->SetValueAsEnum(CAIKey::Engage::AttackActionType, static_cast<uint8>(AttackActionType));

	blackboardComp->SetValueAsBool(CAIKey::Engage::bIsAttacking, true);
	blackboardComp->SetValueAsBool(CAIKey::Engage::bCanAttack, false);

	return EBTNodeResult::Succeeded;
}
