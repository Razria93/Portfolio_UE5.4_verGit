#include "Component/CDefenseComponent.h"
#include "ProjectGlobal.h"

#include "Component/CMovementComponent.h"

UCDefenseComponent::UCDefenseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCDefenseComponent::BeginPlay()
{
	Super::BeginPlay();

	check(GetOwner());

	MovementComp_Cached = GetOwner()->FindComponentByClass<UCMovementComponent>();
	check(MovementComp_Cached);
}

void UCDefenseComponent::WriteObservableOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot) const
{
	OutSnapshot.Guard.bCanStartGuard = bCanStartGuard;
	OutSnapshot.Guard.bWantsGuarding = bWantsGuarding;
	OutSnapshot.Guard.bIsGuardingPose = bIsGuardingPose;
	OutSnapshot.Guard.bCanGuard = bCanGuard;
	OutSnapshot.Guard.bCanParry = bCanParry;
}

bool UCDefenseComponent::CanHandleObservableOverlayEvent(const FObservableOverlayEventContext& InContext) const
{
	switch (InContext.EventType)
	{
	case EObservableOverlayEventType::GuardInputPressed:
	case EObservableOverlayEventType::GuardInputReleased:
	case EObservableOverlayEventType::GuardInStarted:
	case EObservableOverlayEventType::GuardOutStarted:
	case EObservableOverlayEventType::SwitchToGuard:
	case EObservableOverlayEventType::AllowGuardStart:
	case EObservableOverlayEventType::GuardLifecycleCompleted:
	case EObservableOverlayEventType::GuardLifecycleInterrupted:
		return true;

	default:
		return false;
	}
}

bool UCDefenseComponent::HandleObservableOverlayEvent(const FObservableOverlayEventContext& InContext)
{
	if (!CanHandleObservableOverlayEvent(InContext)) return false;

	switch (InContext.EventType)
	{
	case EObservableOverlayEventType::GuardInputPressed:
		HandleGuardInputPressed();
		return true;

	case EObservableOverlayEventType::GuardInputReleased:
		HandleGuardInputReleased();
		return true;

	case EObservableOverlayEventType::GuardInStarted:
		HandleGuardInStarted();
		return true;

	case EObservableOverlayEventType::GuardOutStarted:
		HandleGuardOutStarted();
		return true;

	case EObservableOverlayEventType::SwitchToGuard:
		HandleSwitchToGuard();
		return true;

	case EObservableOverlayEventType::AllowGuardStart:
		HandleAllowGuardStart();
		return true;

	case EObservableOverlayEventType::GuardLifecycleCompleted:
		HandleGuardLifecycleCompleted();
		return true;

	case EObservableOverlayEventType::GuardLifecycleInterrupted:
		HandleGuardLifecycleInterrupted();
		return true;

	default:
		return false;
	}
}

bool UCDefenseComponent::CanApplyObservableOverlayHandling(EObservableOverlayHandling InHandling) const
{
	FLog::Log(FString::Printf(
		TEXT("[DefenseOverlay] CanApply Handling=%s | CanStartGuard=%s | WantsGuarding=%s | IsGuardingPose=%s | CanGuard=%s | CanParry=%s"),
		*UEnum::GetValueAsString(InHandling),
		bCanStartGuard ? TEXT("true") : TEXT("false"),
		bWantsGuarding ? TEXT("true") : TEXT("false"),
		bIsGuardingPose ? TEXT("true") : TEXT("false"),
		bCanGuard ? TEXT("true") : TEXT("false"),
		bCanParry ? TEXT("true") : TEXT("false")));

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
	BeginGuardIntent();
}

void UCDefenseComponent::HandleGuardInputReleased()
{
	EndGuardIntent();
}

void UCDefenseComponent::HandleGuardInStarted()
{
	BlockGuardStart();

	BeginGuardPose();
	CloseGuardWindow();
	OpenParryWindow();

	ApplyGuardMovementOverride();
}

void UCDefenseComponent::HandleGuardOutStarted()
{
	BlockGuardStart();

	EndGuardPose();
	CloseGuardWindow();
	CloseParryWindow();

	ClearMovementOverride();
}

void UCDefenseComponent::HandleSwitchToGuard()
{
	CloseParryWindow();

	if (WantsGuarding())
	{
		OpenGuardWindow();
	}
	else
	{
		CloseGuardWindow();
	}
}

void UCDefenseComponent::HandleAllowGuardStart()
{
	AllowGuardStart();
}

void UCDefenseComponent::HandleGuardLifecycleCompleted()
{
	ClearGuardState();
}

void UCDefenseComponent::HandleGuardLifecycleInterrupted()
{
	ClearGuardState();
}

void UCDefenseComponent::ClearGuardState()
{
	AllowGuardStart();
	EndGuardIntent();

	EndGuardPose();
	CloseGuardWindow();
	CloseParryWindow();

	ClearMovementOverride();
}

void UCDefenseComponent::ClearGuardOverlay()
{
	EndGuardPose();
	CloseGuardWindow();
	CloseParryWindow();

	ClearMovementOverride();
}

void UCDefenseComponent::RestoreGuardOverlay()
{
	if (WantsGuarding())
	{
		BeginGuardPose();
		OpenGuardWindow();
		CloseParryWindow();

		ApplyGuardMovementOverride();
	}
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

void UCDefenseComponent::ApplyGuardMovementOverride()
{
	if (!IsValid(MovementComp_Cached)) return;

	MovementComp_Cached->ApplyMovementOverride(EMovementGait::Walk, EMovementRotationMode::ControllerDesired);
}

void UCDefenseComponent::ClearMovementOverride()
{
	if (!IsValid(MovementComp_Cached)) return;

	MovementComp_Cached->ClearMovementOverride();
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
