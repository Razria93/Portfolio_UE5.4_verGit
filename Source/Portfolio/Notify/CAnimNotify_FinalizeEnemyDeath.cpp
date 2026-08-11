#include "Notify/CAnimNotify_FinalizeEnemyDeath.h"

#include "Character/Enemy/CEnemy.h"

#include "Components/SkeletalMeshComponent.h"

UCAnimNotify_FinalizeEnemyDeath::UCAnimNotify_FinalizeEnemyDeath()
{
}

FString UCAnimNotify_FinalizeEnemyDeath::GetNotifyName_Implementation() const
{
	return TEXT("Finalize Enemy Death");
}

void UCAnimNotify_FinalizeEnemyDeath::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACEnemy* enemy = Cast<ACEnemy>(MeshComp->GetOwner());
	if (!IsValid(enemy)) return;

	enemy->RequestFinalizeDeath();
}
