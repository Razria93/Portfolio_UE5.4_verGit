#include "Notify/CAnimNotify_CombatSignalCue.h"

#include "Component/CActionComponent.h"
#include "Core/Profiling/CCombatCollisionProfilingCounters.h"

UCAnimNotify_CombatSignalCue::UCAnimNotify_CombatSignalCue()
{
}

FString UCAnimNotify_CombatSignalCue::GetNotifyName_Implementation() const
{
	return CueTag.IsNone() ? TEXT("None") : FString::Printf(TEXT("CombatSignalCue: %s"), *CueTag.ToString());
}

void UCAnimNotify_CombatSignalCue::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	if (CueTag.IsNone())
	{
		return;
	}

	FCombatCollisionProfilingCounters::RecordCombatSignalCueNotify();

	actionComp->HandleActionCombatSignalCue(CueTag);
}
