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

// Query
bool UCDebugOverlayTargetComponent::HasDebugOverlayTarget() const
{
	return DebugOverlayTargetActor.IsValid();
}

bool UCDebugOverlayTargetComponent::HasDebugOverlaySelectionSummary() const
{
	return !DebugOverlaySelectionSummary.IsEmpty();
}

AActor* UCDebugOverlayTargetComponent::GetDebugOverlayTargetActor() const
{
	return DebugOverlayTargetActor.Get();
}

FString UCDebugOverlayTargetComponent::GetDebugOverlayTargetSummary() const
{
	return HasDebugOverlayTarget()
		? FString::Printf(TEXT("Selected=%s"), *GetNameSafe(DebugOverlayTargetActor.Get()))
		: FString(TEXT("None"));
}

FString UCDebugOverlayTargetComponent::GetDebugOverlayTargetSource() const
{
	return HasDebugOverlayTarget()
		? FormatDebugOverlayTargetSource(DebugOverlayTargetSource)
		: FString(TEXT("None"));
}

FString UCDebugOverlayTargetComponent::GetDebugOverlaySelectionSummary() const
{
	return DebugOverlaySelectionSummary;
}

// Mutation
void UCDebugOverlayTargetComponent::SetDebugOverlayTarget(AActor* InTargetActor, EDebugOverlayTargetSource InSource)
{
	if (!IsValid(InTargetActor) || InSource == EDebugOverlayTargetSource::None)
	{
		ClearDebugOverlayTarget();
		return;
	}

	DebugOverlayTargetActor = InTargetActor;
	DebugOverlayTargetSource = InSource;
}

void UCDebugOverlayTargetComponent::ClearDebugOverlayTarget()
{
	DebugOverlayTargetActor.Reset();
	DebugOverlayTargetSource = EDebugOverlayTargetSource::None;
	ClearDebugOverlaySelectionSummary();
}

void UCDebugOverlayTargetComponent::SetDebugOverlaySelectionSummary(const FString& InSummary)
{
	DebugOverlaySelectionSummary = InSummary;
}

void UCDebugOverlayTargetComponent::ClearDebugOverlaySelectionSummary()
{
	DebugOverlaySelectionSummary.Reset();
}
