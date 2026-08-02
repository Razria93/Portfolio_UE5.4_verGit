#include "Core/Debug/CDebugOverlayTargetComponent.h"

#include "GameFramework/Actor.h"

namespace
{
	FString FormatDebugOverlayTargetSource(EDebugOverlayTargetSource InSource)
	{
		switch (InSource)
		{
		case EDebugOverlayTargetSource::Nearest:
			return TEXT("TargetComponent.Nearest");
		case EDebugOverlayTargetSource::EditorSelection:
			return TEXT("TargetComponent.EditorSelection");
		case EDebugOverlayTargetSource::None:
		default:
			return TEXT("None");
		}
	}
}

UCDebugOverlayTargetComponent::UCDebugOverlayTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Focus Query
bool UCDebugOverlayTargetComponent::HasDebugOverlayFocus() const
{
	return DebugOverlayTargetActor.IsValid();
}

bool UCDebugOverlayTargetComponent::HasDebugOverlayFocusCommandResult() const
{
	return !DebugOverlaySelectionSummary.IsEmpty();
}

AActor* UCDebugOverlayTargetComponent::GetDebugOverlayFocusActor() const
{
	return DebugOverlayTargetActor.Get();
}

FString UCDebugOverlayTargetComponent::GetDebugOverlayFocusActorText() const
{
	return HasDebugOverlayFocus()
		? FString::Printf(TEXT("Selected: %s"), *GetNameSafe(DebugOverlayTargetActor.Get()))
		: FString(TEXT("None"));
}

FString UCDebugOverlayTargetComponent::GetDebugOverlayFocusModeText() const
{
	return HasDebugOverlayFocus()
		? FormatDebugOverlayTargetSource(DebugOverlayTargetSource)
		: FString(TEXT("None"));
}

FString UCDebugOverlayTargetComponent::GetDebugOverlayFocusCommandResultText() const
{
	return DebugOverlaySelectionSummary;
}

// Focus Mutation
void UCDebugOverlayTargetComponent::SetDebugOverlayFocus(AActor* InFocusActor, EDebugOverlayTargetSource InSource)
{
	if (!IsValid(InFocusActor) || InSource == EDebugOverlayTargetSource::None)
	{
		ClearDebugOverlayFocus();
		return;
	}

	DebugOverlayTargetActor = InFocusActor;
	DebugOverlayTargetSource = InSource;
}

void UCDebugOverlayTargetComponent::ClearDebugOverlayFocus()
{
	DebugOverlayTargetActor.Reset();
	DebugOverlayTargetSource = EDebugOverlayTargetSource::None;
}

void UCDebugOverlayTargetComponent::SetDebugOverlayFocusCommandResult(const FString& InResultText)
{
	DebugOverlaySelectionSummary = InResultText;
}

void UCDebugOverlayTargetComponent::ClearDebugOverlayFocusCommandResult()
{
	DebugOverlaySelectionSummary.Reset();
}

// Compatibility Query
bool UCDebugOverlayTargetComponent::HasDebugOverlayTarget() const
{
	return HasDebugOverlayFocus();
}

bool UCDebugOverlayTargetComponent::HasDebugOverlaySelectionSummary() const
{
	return HasDebugOverlayFocusCommandResult();
}

AActor* UCDebugOverlayTargetComponent::GetDebugOverlayTargetActor() const
{
	return GetDebugOverlayFocusActor();
}

FString UCDebugOverlayTargetComponent::GetDebugOverlayTargetSummary() const
{
	return GetDebugOverlayFocusActorText();
}

FString UCDebugOverlayTargetComponent::GetDebugOverlayTargetSource() const
{
	return GetDebugOverlayFocusModeText();
}

FString UCDebugOverlayTargetComponent::GetDebugOverlaySelectionSummary() const
{
	return GetDebugOverlayFocusCommandResultText();
}

// Compatibility Mutation
void UCDebugOverlayTargetComponent::SetDebugOverlayTarget(AActor* InTargetActor, EDebugOverlayTargetSource InSource)
{
	if (!IsValid(InTargetActor) || InSource == EDebugOverlayTargetSource::None)
	{
		ClearDebugOverlayTarget();
		return;
	}

	SetDebugOverlayFocus(InTargetActor, InSource);
}

void UCDebugOverlayTargetComponent::ClearDebugOverlayTarget()
{
	ClearDebugOverlayFocus();
	ClearDebugOverlayFocusCommandResult();
}

void UCDebugOverlayTargetComponent::SetDebugOverlaySelectionSummary(const FString& InSummary)
{
	SetDebugOverlayFocusCommandResult(InSummary);
}

void UCDebugOverlayTargetComponent::ClearDebugOverlaySelectionSummary()
{
	ClearDebugOverlayFocusCommandResult();
}
