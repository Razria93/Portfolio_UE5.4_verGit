#include "Notify/CAnimNotify_EndEnemyAttack.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCAnimNotify_EndEnemyAttack::UCAnimNotify_EndEnemyAttack()
{
}

FString UCAnimNotify_EndEnemyAttack::GetNotifyName_Implementation() const
{
	return TEXT("End Enemy Attack");
}

void UCAnimNotify_EndEnemyAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!IsValid(MeshComp))
		return;

	APawn* pawn = Cast<APawn>(MeshComp->GetOwner());
	if (!IsValid(pawn)) return;

	AAIController* aiController = Cast<AAIController>(pawn->GetController());
	if (!IsValid(aiController)) return;

	UBlackboardComponent* blackboardComp = aiController->GetBlackboardComponent();
	if (IsValid(blackboardComp))
	{
		blackboardComp->SetValueAsBool(CAIKey::Engage::bIsAttacking, false);

		FLog::Log(TEXT("[EndEnemyAttack_Notify] End Enemy Attack"));
	}
}