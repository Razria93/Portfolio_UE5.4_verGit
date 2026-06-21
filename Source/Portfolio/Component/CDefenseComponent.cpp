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

// Overlay Policy
void UCDefenseComponent::WriteOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot) const
{
	OutSnapshot.Guard.bCanStartGuard = bCanStartGuard;
	OutSnapshot.Guard.bWantsGuarding = bWantsGuarding;
	OutSnapshot.Guard.bIsGuardingPose = bIsGuardingPose;
	OutSnapshot.Guard.bCanGuard = bCanGuard;
	OutSnapshot.Guard.bCanParry = bCanParry;
}

bool UCDefenseComponent::CanApplyOverlayEvent(const FObservableOverlayEventContext& InContext) const
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

bool UCDefenseComponent::ApplyOverlayEvent(const FObservableOverlayEventContext& InContext)
{
	if (!CanApplyOverlayEvent(InContext)) return false;

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

bool UCDefenseComponent::CanApplyOverlayHandling(EObservableOverlayHandling InHandling) const
{
	switch (InHandling)
	{
	case EObservableOverlayHandling::None:
		return true;

	case EObservableOverlayHandling::ClearGuardState:
		return true;

	case EObservableOverlayHandling::ClearGuardOverlay:
		return true;

	default:
		return false;
	}
}

bool UCDefenseComponent::ApplyOverlayHandling(EObservableOverlayHandling InHandling)
{
	if (!CanApplyOverlayHandling(InHandling)) return false;

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

// Guard Event Entry
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

// Guard State Cleanup
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

// Guard State Primitive
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

// Movement Override
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

// Debug
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
