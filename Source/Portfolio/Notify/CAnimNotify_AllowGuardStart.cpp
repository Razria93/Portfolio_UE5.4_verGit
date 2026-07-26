#include "Notify/CAnimNotify_AllowGuardStart.h"

#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"
#include "Type/CActionKeyTypes.h"

UCAnimNotify_AllowGuardStart::UCAnimNotify_AllowGuardStart()
{
	TriggerActionType = EActionType::Guard;
	TriggerActionIndex = GetGuardActionPhaseIndex(EGuardActionPhase::Out);
}

FString UCAnimNotify_AllowGuardStart::GetNotifyName_Implementation() const
{
	return TEXT("Allow Guard Start");
}

void UCAnimNotify_AllowGuardStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionNotifyCommand(EActionNotifyCommand::AllowGuardStart);
}
