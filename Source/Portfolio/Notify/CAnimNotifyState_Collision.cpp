#include "Notify/CAnimNotifyState_Collision.h"
#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"
#include "Core/Profiling/CCombatCollisionProfilingCounters.h"

UCAnimNotifyState_Collision::UCAnimNotifyState_Collision()
{
}

FString UCAnimNotifyState_Collision::GetNotifyName_Implementation() const
{
	return CollisionName.IsNone() ? TEXT("Collision Window") : FString::Printf(TEXT("Collision Window: %s"), *CollisionName.ToString());
}

void UCAnimNotifyState_Collision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	FCombatCollisionProfilingCounters::RecordCollisionNotifyBegin();

	actionComp->HandleActionCollisionWindowBegin(CollisionName);
}

void UCAnimNotifyState_Collision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	FCombatCollisionProfilingCounters::RecordCollisionNotifyEnd();

	actionComp->HandleActionCollisionWindowEnd();
}
