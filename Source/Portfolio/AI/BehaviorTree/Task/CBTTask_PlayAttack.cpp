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
	if (!IsValid(AttackMontage))
		return EBTNodeResult::Failed;

	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACharacter* character = Cast<ACharacter>(aiController->GetPawn());
	if (!IsValid(character)) return EBTNodeResult::Failed;

	USkeletalMeshComponent* meshComp = character->GetMesh();
	if (!IsValid(meshComp)) return EBTNodeResult::Failed;

	UAnimInstance* animInstance = meshComp->GetAnimInstance();
	if (!IsValid(animInstance)) return EBTNodeResult::Failed;

	FName attackSection = NAME_None; // OutParameter
	if (!ResolveAttackSection(blackboardComp, attackSection))
		return EBTNodeResult::Failed;

	const float duration = animInstance->Montage_Play(AttackMontage);
	if (duration <= 0.f) return EBTNodeResult::Failed;

	if (attackSection != NAME_None)
	{
		animInstance->Montage_JumpToSection(attackSection, AttackMontage);
	}

	return EBTNodeResult::Succeeded;
}

bool UCBTTask_PlayAttack::ResolveAttackSection(UBlackboardComponent* InBlackboardComp, FName& OutSectionName) const
{
	if (!IsValid(InBlackboardComp)) return false;

	const int32 attackIndex = InBlackboardComp->GetValueAsInt(CAIKey::Combat::AttackIndex);
	if (!AttackSections.IsValidIndex(attackIndex)) return false;

	OutSectionName = AttackSections[attackIndex];
	return true;
}