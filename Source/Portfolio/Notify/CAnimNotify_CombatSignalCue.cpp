#include "Notify/CAnimNotify_CombatSignalCue.h"
#include "ProjectGlobal.h"

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
		// FLog::Log(TEXT("[CombatSignalCueNotify] Rejected | Reason=InvalidCueTag"));
		return;
	}

	FCombatCollisionProfilingCounters::RecordCombatSignalCueNotify();

	const bool bSent = actionComp->HandleActionCombatSignalCue(CueTag);

	// FLog::Log(FString::Printf(
	// 	TEXT("[CombatSignalCueNotify] %s | Source=%s | CueTag=%s"),
	// 	bSent ? TEXT("Sent") : TEXT("Rejected"),
	// 	*GetNameSafe(IsValid(MeshComp) ? MeshComp->GetOwner() : nullptr),
	// 	*CueTag.ToString()));
}
