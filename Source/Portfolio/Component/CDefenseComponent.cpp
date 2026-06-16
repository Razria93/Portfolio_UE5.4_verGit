#include "Component/CDefenseComponent.h"
#include "ProjectGlobal.h"

UCDefenseComponent::UCDefenseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCDefenseComponent::BeginGuardIntent()
{
	bWantsGuarding = true;
}

void UCDefenseComponent::EndGuardIntent()
{
	bWantsGuarding = false;
}

void UCDefenseComponent::BeginGuardPose()
{
	bIsGuardingPose = true;
}

void UCDefenseComponent::EndGuardPose()
{
	bIsGuardingPose = false;
}

void UCDefenseComponent::OpenGuardWindow()
{
	bCanGuard = true;
}

void UCDefenseComponent::CloseGuardWindow()
{
	bCanGuard = false;
}

void UCDefenseComponent::OpenParryWindow()
{
	bCanParry = true;
}

void UCDefenseComponent::CloseParryWindow()
{
	bCanParry = false;
}

void UCDefenseComponent::ResetGuardState()
{
	bWantsGuarding = false;
	bIsGuardingPose = false;
	bCanGuard = false;
	bCanParry = false;
}

void UCDefenseComponent::ClearGuardOverlay()
{
	bIsGuardingPose = false;
	bCanGuard = false;
	bCanParry = false;
}

void UCDefenseComponent::WriteObservableOverlayState(FObservableOverlayState& InOutOverlayState) const
{
	InOutOverlayState.bIsGuardingPose = IsGuardingPose();
	InOutOverlayState.bCanGuard = CanGuard();
	InOutOverlayState.bCanParry = CanParry();
}

bool UCDefenseComponent::HasRelevantOverlay(const FExecutionSnapshot& InSnapshot) const
{
	return InSnapshot.HasGuardOverlay();
}

void UCDefenseComponent::ResolveObservableOverlayDecision(const FObservableOverlayQuery& InQuery, FObservableOverlayDecision& OutDecision) const
{
	OutDecision = FObservableOverlayDecision();

	if (!HasRelevantOverlay(InQuery.Snapshot)) return;

	const bool bNeedsExecutionStart = InQuery.ApplyMode == EExecutionApplyMode::Start || InQuery.ApplyMode == EExecutionApplyMode::Intervene;
	if (!bNeedsExecutionStart) return;

	if (InQuery.IncomingPart.IsReactionParticipant())
	{
		OutDecision.Handling = EObservableOverlayHandling::ClearGuardOverlayBeforeStart;
		return;
	}

	if (!InQuery.IncomingPart.IsActionParticipant())
	{
		OutDecision.bAllowed = false;
		return;
	}

	const FActionDataKey& incomingActionKey = InQuery.IncomingPart.GetActionContext().ActionDataKey;

	const bool bIsGuardOut = incomingActionKey.ActionType == EActionType::Guard && incomingActionKey.ActionIndex == 2;
	const bool bIsDodge = incomingActionKey.ActionType == EActionType::Dodge;

	if (!bIsGuardOut && !bIsDodge)
	{
		OutDecision.bAllowed = false;
		return;
	}

	OutDecision.Handling = EObservableOverlayHandling::ClearGuardOverlayBeforeStart;
}

void UCDefenseComponent::HandleGuardInStarted()
{
	BeginGuardIntent();
	BeginGuardPose();

	CloseGuardWindow();
	OpenParryWindow();
	
	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardOutStarted()
{
	EndGuardIntent();
	EndGuardPose();

	CloseGuardWindow();
	CloseParryWindow();
	
	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardOutCompleted()
{
	ResetGuardState();

	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardInterrupted(EActionStopReason InStopReason)
{
	ResetGuardState();

	PrintGuardStateInfo();
}

void UCDefenseComponent::PrintGuardStateInfo() const
{
	FLog::Log(FString::Printf(
		TEXT("[Defense] WantsGuarding = %s | IsGuardingPose = %s | CanGuard = %s | CanParry = %s"),
		bWantsGuarding ? TEXT("true") : TEXT("false"),
		bIsGuardingPose ? TEXT("true") : TEXT("false"),
		bCanGuard ? TEXT("true") : TEXT("false"),
		bCanParry ? TEXT("true") : TEXT("false")));
}
