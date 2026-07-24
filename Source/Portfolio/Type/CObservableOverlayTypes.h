#pragma once

#include "CoreMinimal.h"
#include "CObservableOverlayTypes.generated.h"

UENUM(BlueprintType)
enum class EObservableOverlayHandling : uint8
{
	None = 0,

	ClearGuardState,
	ClearGuardOverlay,

	Max,
};

UENUM(BlueprintType)
enum class EObservableOverlayEventType : uint8
{
	None = 0,

	GuardInputPressed,
	GuardInputReleased,

	GuardInStarted,
	GuardOutStarted,

	SwitchToGuard,
	AllowGuardStart,

	GuardLifecycleCompleted,
	GuardLifecycleInterrupted,

	Max,
};

USTRUCT(BlueprintType)
struct FObservableOverlayEventContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EObservableOverlayEventType EventType = EObservableOverlayEventType::None;

public:
	FObservableOverlayEventContext() = default;

	explicit FObservableOverlayEventContext(EObservableOverlayEventType InEventType)
		: EventType(InEventType)
	{
	}

public:
	bool IsValidMinimal() const
	{
		return EventType != EObservableOverlayEventType::None && EventType != EObservableOverlayEventType::Max;
	}
};

USTRUCT(BlueprintType)
struct FGuardObservableOverlaySnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bWantsGuarding = false;

	UPROPERTY(Transient)
	bool bIsGuardingPose = false;

	UPROPERTY(Transient)
	bool bCanGuard = false;

	UPROPERTY(Transient)
	bool bCanParry = false;

	UPROPERTY(Transient)
	bool bCanStartGuard = true;

public:
	bool HasGuardOverlay() const
	{
		return bIsGuardingPose || bCanGuard || bCanParry;
	}

	bool HasGuardRuntimeState() const
	{
		return !bCanStartGuard || bWantsGuarding || HasGuardOverlay();
	}
};

USTRUCT(BlueprintType)
struct FObservableOverlaySnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FGuardObservableOverlaySnapshot Guard = FGuardObservableOverlaySnapshot();

public:
	bool HasObservableOverlay() const
	{
		return Guard.HasGuardOverlay();
	}
};
