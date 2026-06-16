#include "Component/CDefenseComponent.h"
#include "ProjectGlobal.h"

UCDefenseComponent::UCDefenseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCDefenseComponent::AllowGuardStart()
{
	bCanStartGuard = true;
}

void UCDefenseComponent::BlockGuardStart()
{
	bCanStartGuard = false;
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
	bCanStartGuard = true;
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

void UCDefenseComponent::WriteObservableOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot) const
{
	OutSnapshot.Guard.bCanStartGuard = bCanStartGuard;
	OutSnapshot.Guard.bWantsGuarding = bWantsGuarding;
	OutSnapshot.Guard.bIsGuardingPose = bIsGuardingPose;
	OutSnapshot.Guard.bCanGuard = bCanGuard;
	OutSnapshot.Guard.bCanParry = bCanParry;
}

bool UCDefenseComponent::CanApplyObservableOverlayHandling(EObservableOverlayHandling InHandling) const
{
	switch (InHandling)
	{
	case EObservableOverlayHandling::None:
		return true;

	case EObservableOverlayHandling::ClearGuardOverlay:
		return HasGuardOverlay();

	default:
		return false;
	}
}

bool UCDefenseComponent::ApplyObservableOverlayHandling(EObservableOverlayHandling InHandling)
{
	if (!CanApplyObservableOverlayHandling(InHandling)) return false;

	switch (InHandling)
	{
	case EObservableOverlayHandling::None:
		return true;

	case EObservableOverlayHandling::ClearGuardOverlay:
		ClearGuardOverlay();
		return true;

	default:
		return false;
	}
}

void UCDefenseComponent::HandleGuardInStarted()
{
	FLog::Log(TEXT("[HandleGuardInStarted]"));

	BlockGuardStart();

	BeginGuardIntent();
	BeginGuardPose();

	CloseGuardWindow();
	OpenParryWindow();
	
	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardOutStarted()
{
	FLog::Log(TEXT("[HandleGuardOutStarted]"));

	AllowGuardStart();

	EndGuardIntent();
	EndGuardPose();

	CloseGuardWindow();
	CloseParryWindow();
	
	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleSwitchToGuard()
{
	FLog::Log(TEXT("[HandleSwitchToGuard]"));

	CloseParryWindow();

	if (WantsGuarding())
	{
		FLog::Log(TEXT("[WantsGuarding == true]"));
		OpenGuardWindow();
	}
	else
	{
		FLog::Log(TEXT("[WantsGuarding == false]"));
		CloseGuardWindow();
	}

	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardOutCompleted()
{
	FLog::Log(TEXT("[HandleGuardOutCompleted]"));

	ResetGuardState();

	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardInterrupted(EActionStopReason InStopReason)
{
	FLog::Log(TEXT("[HandleGuardInterrupted]"));

	ResetGuardState();

	PrintGuardStateInfo();
}

void UCDefenseComponent::PrintGuardStateInfo() const
{
	FLog::Log(FString::Printf(
		TEXT("[Defense] CanStartGuard = %s | WantsGuarding = %s | IsGuardingPose = %s | CanGuard = %s | CanParry = %s"),
		bCanStartGuard ? TEXT("true") : TEXT("false"),
		bWantsGuarding ? TEXT("true") : TEXT("false"),
		bIsGuardingPose ? TEXT("true") : TEXT("false"),
		bCanGuard ? TEXT("true") : TEXT("false"),
		bCanParry ? TEXT("true") : TEXT("false")));
}
