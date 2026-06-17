#include "Component/CDefenseComponent.h"
#include "ProjectGlobal.h"

UCDefenseComponent::UCDefenseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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

	case EObservableOverlayHandling::ClearGuardState:
		return HasGuardRuntimeState();

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

	case EObservableOverlayHandling::ClearGuardState:
		ClearGuardState();
		return true;

	case EObservableOverlayHandling::ClearGuardOverlay:
		ClearGuardOverlay();
		return true;

	default:
		return false;
	}
}

void UCDefenseComponent::HandleGuardInputPressed()
{
	FLog::Log(TEXT("[HandleGuardInputPressed]"));

	BeginGuardIntent();

	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardInputReleased()
{
	FLog::Log(TEXT("[HandleGuardInputReleased]"));

	EndGuardIntent();

	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardInStarted()
{
	FLog::Log(TEXT("[HandleGuardInStarted]"));

	BlockGuardStart();

	BeginGuardPose();
	CloseGuardWindow();
	OpenParryWindow();

	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardOutStarted()
{
	FLog::Log(TEXT("[HandleGuardOutStarted]"));

	BlockGuardStart();

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

void UCDefenseComponent::HandleAllowGuardStart()
{
	FLog::Log(TEXT("[HandleAllowGuardStart]"));

	AllowGuardStart();

	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardLifecycleCompleted()
{
	FLog::Log(TEXT("[HandleGuardLifecycleCompleted]"));

	ClearGuardState();

	PrintGuardStateInfo();
}

void UCDefenseComponent::HandleGuardLifecycleInterrupted()
{
	FLog::Log(TEXT("[HandleGuardLifecycleInterrupted]"));

	ClearGuardState();

	PrintGuardStateInfo();
}

void UCDefenseComponent::ClearGuardState()
{
	FLog::Log(TEXT("[ClearGuardState]"));

	AllowGuardStart();
	EndGuardIntent();

	EndGuardPose();
	CloseGuardWindow();
	CloseParryWindow();

	PrintGuardStateInfo();
}

void UCDefenseComponent::ClearGuardOverlay()
{
	FLog::Log(TEXT("[ClearGuardOverlay]"));

	EndGuardPose();
	CloseGuardWindow();
	CloseParryWindow();

	PrintGuardStateInfo();
}

void UCDefenseComponent::RestoreGuardOverlay()
{
	FLog::Log(TEXT("[RestoreGuardOverlay]"));

	if (WantsGuarding())
	{
		BeginGuardPose();
		OpenGuardWindow();
		CloseParryWindow();
	}

	PrintGuardStateInfo();
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
