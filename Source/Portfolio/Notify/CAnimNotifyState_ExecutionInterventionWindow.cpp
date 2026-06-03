#include "Notify/CAnimNotifyState_ExecutionInterventionWindow.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"

UCAnimNotifyState_ExecutionInterventionWindow::UCAnimNotifyState_ExecutionInterventionWindow()
{
}

FString UCAnimNotifyState_ExecutionInterventionWindow::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("Intervention Window (%s)"), *WindowKey.ToString());
}

FLinearColor UCAnimNotifyState_ExecutionInterventionWindow::GetEditorColor()
{
	return FLinearColor(0.1f, 0.45f, 0.95f, 1.0f);
}

void UCAnimNotifyState_ExecutionInterventionWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	HandleWindow(MeshComp, true);
}

void UCAnimNotifyState_ExecutionInterventionWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	HandleWindow(MeshComp, false);
}

void UCAnimNotifyState_ExecutionInterventionWindow::HandleWindow(USkeletalMeshComponent* InMeshComp, bool bOpen) const
{
	if (WindowKey.IsNone()) return;
	if (!IsValid(InMeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(InMeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	if (UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>())
	{
		if (bOpen)
		{
			actionComp->HandleActionInterventionWindowBegin(WindowKey);
		}
		else
		{
			actionComp->HandleActionInterventionWindowEnd(WindowKey);
		}
	}

	if (UCReactionComponent* reactionComp = ownerCharacter->FindComponentByClass<UCReactionComponent>())
	{
		if (bOpen)
		{
			reactionComp->HandleReactionInterventionWindowBegin(WindowKey);
		}
		else
		{
			reactionComp->HandleReactionInterventionWindowEnd(WindowKey);
		}
	}
}
