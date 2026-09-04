#include "Core/Debug/CDebugOverlayFocusComponent.h"

#include "GameFramework/Actor.h"

namespace
{
	FString FormatDebugOverlayFocusSource(EDebugOverlayFocusSource InSource)
	{
		switch (InSource)
		{
		case EDebugOverlayFocusSource::NearestFocus:
			return TEXT("FocusComponent.NearestFocus");
		case EDebugOverlayFocusSource::RecentCombatFocus:
			return TEXT("FocusComponent.RecentCombatFocus");
		case EDebugOverlayFocusSource::WorldScanFallback:
			return TEXT("FocusComponent.WorldScanFallback");
		case EDebugOverlayFocusSource::PlayerTargetFocus:
			return TEXT("FocusComponent.PlayerTargetFocus");
		case EDebugOverlayFocusSource::OutlinerFocus:
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
		case EDebugOverlayFocusDriver::PlayerTargetLive:
			return TEXT("PlayerTargetLive");
		case EDebugOverlayFocusDriver::PlayerTargetFrozen:
			return TEXT("PlayerTargetFrozen");
		case EDebugOverlayFocusDriver::FocusComponentLive:
			return TEXT("FocusComponentLive");
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
		case EDebugOverlayRecentFocusState::NoFocusFound:
			return TEXT("NoFocusFound");
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
