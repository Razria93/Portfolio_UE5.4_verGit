#include "Core/Debug/CDebugOverlayFocusComponent.h"

#include "GameFramework/Actor.h"

namespace
{
	FString FormatDebugOverlayFocusSource(EDebugOverlayFocusSource InSource)
	{
		switch (InSource)
		{
		case EDebugOverlayFocusSource::NearestTarget:
			return TEXT("FocusComponent.NearestFocus");
		case EDebugOverlayFocusSource::RecentCombat:
			return TEXT("FocusComponent.RecentCombat");
		case EDebugOverlayFocusSource::WorldScanFallback:
			return TEXT("FocusComponent.WorldScanFallback");
		case EDebugOverlayFocusSource::GameplayTarget:
			return TEXT("FocusComponent.GameplayTarget");
		case EDebugOverlayFocusSource::OutlinerTarget:
			return TEXT("FocusComponent.OutlinerFocus");
		case EDebugOverlayFocusSource::None:
		default:
			return TEXT("None");
		}
	}

	FString FormatDebugOverlayFocusDriver(EDebugOverlayFocusDriver InDriver)
	{
		switch (InDriver)
		{
		case EDebugOverlayFocusDriver::ManualNearest:
			return TEXT("NearestFocus");
		case EDebugOverlayFocusDriver::ManualOutliner:
			return TEXT("OutlinerFocus");
		case EDebugOverlayFocusDriver::RecentCombatLive:
			return TEXT("RecentCombatLive");
		case EDebugOverlayFocusDriver::TargetComponentLive:
			return TEXT("TargetComponentLive");
		case EDebugOverlayFocusDriver::None:
		default:
			return TEXT("None");
		}
	}

	FString FormatDebugOverlayRecentFocusState(EDebugOverlayRecentFocusState InState)
	{
		switch (InState)
		{
		case EDebugOverlayRecentFocusState::Selected:
			return TEXT("Selected");
		case EDebugOverlayRecentFocusState::NoTargetFound:
			return TEXT("NoTargetFound");
		case EDebugOverlayRecentFocusState::NoRecentCombatEvidence:
			return TEXT("NoRecentCombatEvidence");
		case EDebugOverlayRecentFocusState::ClosestOutOfRange:
			return TEXT("ClosestOutOfRange");
		case EDebugOverlayRecentFocusState::None:
		default:
			return TEXT("None");
		}
	}

}

UCDebugOverlayFocusComponent::UCDebugOverlayFocusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Focus Value Query
bool UCDebugOverlayFocusComponent::HasDebugOverlayFocus() const
{
	return DebugOverlayFocusActor.IsValid();
}

AActor* UCDebugOverlayFocusComponent::GetDebugOverlayFocusActor() const
{
	return DebugOverlayFocusActor.Get();
}

EDebugOverlayFocusSource UCDebugOverlayFocusComponent::GetDebugOverlayFocusSource() const
{
	return DebugOverlayFocusSource;
}

EDebugOverlayFocusDriver UCDebugOverlayFocusComponent::GetDebugOverlayFocusDriver() const
{
	return DebugOverlayFocusDriver;
}

// Focus Text Query
FString UCDebugOverlayFocusComponent::GetDebugOverlayFocusActorText() const
{
	return HasDebugOverlayFocus()
		? FString::Printf(TEXT("Selected: %s"), *GetNameSafe(DebugOverlayFocusActor.Get()))
		: FString(TEXT("None"));
}

FString UCDebugOverlayFocusComponent::GetDebugOverlayFocusActorNameText() const
{
	return HasDebugOverlayFocus()
		? GetNameSafe(DebugOverlayFocusActor.Get())
		: FString(TEXT("None"));
}

FString UCDebugOverlayFocusComponent::GetDebugOverlayFocusSourceText() const
{
	return HasDebugOverlayFocus()
		? FormatDebugOverlayFocusSource(DebugOverlayFocusSource)
		: FString(TEXT("None"));
}

FString UCDebugOverlayFocusComponent::GetDebugOverlayFocusDriverText() const
{
	return FormatDebugOverlayFocusDriver(DebugOverlayFocusDriver);
}

EDebugOverlayRecentFocusState UCDebugOverlayFocusComponent::GetDebugOverlayRecentFocusState() const
{
	return DebugOverlayRecentFocusState;
}

FString UCDebugOverlayFocusComponent::GetDebugOverlayRecentFocusStateText() const
{
	return FormatDebugOverlayRecentFocusState(DebugOverlayRecentFocusState);
}

// Focus Mutation
void UCDebugOverlayFocusComponent::SetDebugOverlayFocusActorAndSource(AActor* InFocusActor, EDebugOverlayFocusSource InSource)
{
	if (!IsValid(InFocusActor) || InSource == EDebugOverlayFocusSource::None)
	{
		ClearDebugOverlayFocusActorAndSource();
		return;
	}

	DebugOverlayFocusActor = InFocusActor;
	DebugOverlayFocusSource = InSource;
}

void UCDebugOverlayFocusComponent::SetDebugOverlayFocusDriver(EDebugOverlayFocusDriver InDriver)
{
	DebugOverlayFocusDriver = InDriver;
}

void UCDebugOverlayFocusComponent::SetDebugOverlayRecentFocusState(EDebugOverlayRecentFocusState InState)
{
	DebugOverlayRecentFocusState = InState;
}

void UCDebugOverlayFocusComponent::ClearDebugOverlayFocusActorAndSource()
{
	DebugOverlayFocusActor.Reset();
	DebugOverlayFocusSource = EDebugOverlayFocusSource::None;
}

void UCDebugOverlayFocusComponent::ClearDebugOverlayFocusDriver()
{
	DebugOverlayFocusDriver = EDebugOverlayFocusDriver::None;
}

void UCDebugOverlayFocusComponent::ClearDebugOverlayRecentFocusState()
{
	DebugOverlayRecentFocusState = EDebugOverlayRecentFocusState::None;
}
