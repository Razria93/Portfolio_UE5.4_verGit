#include "Core/Debug/CDebugOverlayFocusComponent.h"

#include "GameFramework/Actor.h"

namespace
{
	FString FormatDebugOverlayFocusSource(EDebugOverlayFocusSource InSource)
	{
		switch (InSource)
		{
		case EDebugOverlayFocusSource::Nearest:
			return TEXT("FocusComponent.Nearest");
		case EDebugOverlayFocusSource::EditorSelection:
			return TEXT("FocusComponent.EditorSelection");
		case EDebugOverlayFocusSource::None:
		default:
			return TEXT("None");
		}
	}
}

UCDebugOverlayFocusComponent::UCDebugOverlayFocusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Focus Query
bool UCDebugOverlayFocusComponent::HasDebugOverlayFocus() const
{
	return DebugOverlayFocusActor.IsValid();
}

bool UCDebugOverlayFocusComponent::HasDebugOverlayFocusCommandResult() const
{
	return !DebugOverlayFocusCommandResult.IsEmpty();
}

AActor* UCDebugOverlayFocusComponent::GetDebugOverlayFocusActor() const
{
	return DebugOverlayFocusActor.Get();
}

FString UCDebugOverlayFocusComponent::GetDebugOverlayFocusActorText() const
{
	return HasDebugOverlayFocus()
		? FString::Printf(TEXT("Selected: %s"), *GetNameSafe(DebugOverlayFocusActor.Get()))
		: FString(TEXT("None"));
}

FString UCDebugOverlayFocusComponent::GetDebugOverlayFocusModeText() const
{
	return HasDebugOverlayFocus()
		? FormatDebugOverlayFocusSource(DebugOverlayFocusSource)
		: FString(TEXT("None"));
}

FString UCDebugOverlayFocusComponent::GetDebugOverlayFocusCommandResultText() const
{
	return DebugOverlayFocusCommandResult;
}

// Focus Mutation
void UCDebugOverlayFocusComponent::SetDebugOverlayFocus(AActor* InFocusActor, EDebugOverlayFocusSource InSource)
{
	if (!IsValid(InFocusActor) || InSource == EDebugOverlayFocusSource::None)
	{
		ClearDebugOverlayFocus();
		return;
	}

	DebugOverlayFocusActor = InFocusActor;
	DebugOverlayFocusSource = InSource;
}

void UCDebugOverlayFocusComponent::ClearDebugOverlayFocus()
{
	DebugOverlayFocusActor.Reset();
	DebugOverlayFocusSource = EDebugOverlayFocusSource::None;
}

void UCDebugOverlayFocusComponent::SetDebugOverlayFocusCommandResult(const FString& InResultText)
{
	DebugOverlayFocusCommandResult = InResultText;
}

void UCDebugOverlayFocusComponent::ClearDebugOverlayFocusCommandResult()
{
	DebugOverlayFocusCommandResult.Reset();
}

// Compatibility Query
bool UCDebugOverlayFocusComponent::HasDebugOverlayTarget() const
{
	return HasDebugOverlayFocus();
}

bool UCDebugOverlayFocusComponent::HasDebugOverlaySelectionSummary() const
{
	return HasDebugOverlayFocusCommandResult();
}

AActor* UCDebugOverlayFocusComponent::GetDebugOverlayTargetActor() const
{
	return GetDebugOverlayFocusActor();
}

FString UCDebugOverlayFocusComponent::GetDebugOverlayTargetSummary() const
{
	return GetDebugOverlayFocusActorText();
}

FString UCDebugOverlayFocusComponent::GetDebugOverlayTargetSource() const
{
	return GetDebugOverlayFocusModeText();
}

FString UCDebugOverlayFocusComponent::GetDebugOverlaySelectionSummary() const
{
	return GetDebugOverlayFocusCommandResultText();
}

// Compatibility Mutation
void UCDebugOverlayFocusComponent::SetDebugOverlayTarget(AActor* InTargetActor, EDebugOverlayFocusSource InSource)
{
	if (!IsValid(InTargetActor) || InSource == EDebugOverlayFocusSource::None)
	{
		ClearDebugOverlayTarget();
		return;
	}

	SetDebugOverlayFocus(InTargetActor, InSource);
}

void UCDebugOverlayFocusComponent::ClearDebugOverlayTarget()
{
	ClearDebugOverlayFocus();
	ClearDebugOverlayFocusCommandResult();
}

void UCDebugOverlayFocusComponent::SetDebugOverlaySelectionSummary(const FString& InSummary)
{
	SetDebugOverlayFocusCommandResult(InSummary);
}

void UCDebugOverlayFocusComponent::ClearDebugOverlaySelectionSummary()
{
	ClearDebugOverlayFocusCommandResult();
}
