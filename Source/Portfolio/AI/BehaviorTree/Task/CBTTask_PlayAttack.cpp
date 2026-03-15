#include "AI/BehaviorTree/Task/CBTTask_PlayAttack.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_PlayAttack::UCBTTask_PlayAttack()
{
	NodeName = TEXT("Play Attack");
}

EBTNodeResult::Type UCBTTask_PlayAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	const int32 attackIndex = blackboardComp->GetValueAsInt(CAIKey::Combat::AttackIndex);
	if (!AttackMontages.IsValidIndex(attackIndex)) return EBTNodeResult::Failed;

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

	const float duration = animInstance->Montage_Play(attackMontage);
	if (duration <= 0.f) return EBTNodeResult::Failed;

	return EBTNodeResult::Succeeded;
}
