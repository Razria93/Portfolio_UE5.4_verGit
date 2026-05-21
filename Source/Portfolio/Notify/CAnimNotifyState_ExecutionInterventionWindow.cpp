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
	return FString::Printf(TEXT("Intervention Window (%s %s)"), *StopReasonToText(), *WindowRoleToText());
}

FLinearColor UCAnimNotifyState_ExecutionInterventionWindow::GetEditorColor()
{
	if (StopReason == EExecutionStopReason::Cancelled && WindowRole == EExecutionInterventionWindowRole::Allow)
	{
		// Cancelled Allow: Green
		return FLinearColor(0.1f, 0.75f, 0.25f, 1.0f);
	}

	if (StopReason == EExecutionStopReason::Cancelled && WindowRole == EExecutionInterventionWindowRole::Want)
	{
		// Cancelled Want: Yellow
		return FLinearColor(0.95f, 0.75f, 0.1f, 1.0f);
	}

	if (StopReason == EExecutionStopReason::Interrupted && WindowRole == EExecutionInterventionWindowRole::Allow)
	{
		// Interrupted Allow: Blue
		return FLinearColor(0.1f, 0.45f, 0.95f, 1.0f);
	}

	if (StopReason == EExecutionStopReason::Interrupted && WindowRole == EExecutionInterventionWindowRole::Want)
	{
		// Interrupted Want: Red
		return FLinearColor(0.95f, 0.25f, 0.1f, 1.0f);
	}

	// Fallback: Gray
	return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
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
	if (!OwnerFilter.IsValidMinimal()) return;
	if (StopReason == EExecutionStopReason::None || StopReason == EExecutionStopReason::Max) return;
	if (WindowRole == EExecutionInterventionWindowRole::None || WindowRole == EExecutionInterventionWindowRole::Max) return;
	if (CounterpartFilters.IsEmpty()) return;
	if (!IsValid(InMeshComp)) return;

	TArray<FExecutionInterventionParticipantFilter> validCounterpartFilters;
	for (const FExecutionInterventionParticipantFilter& filter : CounterpartFilters)
	{
		if (!filter.IsValidMinimal()) continue;

		validCounterpartFilters.Add(filter);
	}

	if (validCounterpartFilters.IsEmpty()) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(InMeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	switch (OwnerFilter.Domain)
	{
	case EExecutionDomain::Action:
	{
		UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
		if (!IsValid(actionComp)) return;

		if (bOpen)
		{
			actionComp->HandleActionInterventionWindowBegin(OwnerFilter, StopReason, WindowRole, validCounterpartFilters);
		}
		else
		{
			actionComp->HandleActionInterventionWindowEnd(OwnerFilter, StopReason, WindowRole, validCounterpartFilters);
		}

		return;
	}

	case EExecutionDomain::Reaction:
	{
		UCReactionComponent* reactionComp = ownerCharacter->FindComponentByClass<UCReactionComponent>();
		if (!IsValid(reactionComp)) return;

		if (bOpen)
		{
			reactionComp->HandleReactionInterventionWindowBegin(OwnerFilter, StopReason, WindowRole, validCounterpartFilters);
		}
		else
		{
			reactionComp->HandleReactionInterventionWindowEnd(OwnerFilter, StopReason, WindowRole, validCounterpartFilters);
		}

		return;
	}

	default:
		return;
	}
}

FString UCAnimNotifyState_ExecutionInterventionWindow::StopReasonToText() const
{
	switch (StopReason)
	{
	case EExecutionStopReason::Cancelled:
		return TEXT("Cancel");

	case EExecutionStopReason::Interrupted:
		return TEXT("Interrupt");

	default:
		return TEXT("Invalid");
	}
}

FString UCAnimNotifyState_ExecutionInterventionWindow::WindowRoleToText() const
{
	switch (WindowRole)
	{
	case EExecutionInterventionWindowRole::Want:
		return TEXT("Want");

	case EExecutionInterventionWindowRole::Allow:
		return TEXT("Allow");

	default:
		return TEXT("Invalid");
	}
}
