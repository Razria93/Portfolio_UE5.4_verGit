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

void UCDefenseComponent::ResolveObservableOverlayDecision(const FObservableOverlayQuery& InQuery, FObservableOverlayDecision& OutDecision) const
{
	OutDecision = FObservableOverlayDecision();

	if (!HasGuardOverlay()) return;

	OutDecision.bRelevant = true;

	if (!NeedsObservableOverlayGate(InQuery.ApplyMode)) return;

	const FExecutionParticipant& incomingPart = InQuery.DecisionQuery.IncomingPart;

	if (incomingPart.IsReactionParticipant())
	{
		ResolveGuardOverlayForReaction(incomingPart.GetReactionContext(), OutDecision);
		return;
	}

	if (incomingPart.IsActionParticipant())
	{
		ResolveGuardOverlayForAction(incomingPart.GetActionContext(), OutDecision);
		return;
	}

	OutDecision.bAllowed = false;
}

bool UCDefenseComponent::NeedsObservableOverlayGate(EExecutionApplyMode InApplyMode) const
{
	return InApplyMode == EExecutionApplyMode::Start || InApplyMode == EExecutionApplyMode::Intervene;
}

void UCDefenseComponent::ResolveGuardOverlayForAction(const FActionExecutionContext& InIncomingContext, FObservableOverlayDecision& OutDecision) const
{
	const FActionDataKey& incomingActionKey = InIncomingContext.ActionDataKey;
	const bool bIsGuardOut = incomingActionKey.ActionType == EActionType::Guard && incomingActionKey.ActionIndex == 2;
	const bool bIsDodge = incomingActionKey.ActionType == EActionType::Dodge;

	if (!bIsGuardOut && !bIsDodge)
	{
		OutDecision.bAllowed = false;
		return;
	}

	OutDecision.Handlings.AddUnique(EObservableOverlayHandling::ClearGuardOverlay);
}

void UCDefenseComponent::ResolveGuardOverlayForReaction(const FReactionExecutionContext& InIncomingContext, FObservableOverlayDecision& OutDecision) const
{
	const FReactionDataKey& incomingReactionKey = InIncomingContext.ReactionDataKey;

	// Temporary: keep Guard overlay until Block_Hit / GuardBreak reaction types are separated.
	switch (incomingReactionKey.ReactionType)
	{
	case EReactionType::Hit:
	case EReactionType::Dead:
		OutDecision.Handlings.AddUnique(EObservableOverlayHandling::ClearGuardOverlay);
		return;

	default:
		OutDecision.bAllowed = false;
		return;
	}
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
