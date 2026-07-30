#include "Core/Debug/CDebugOverlayTargetComponent.h"

#include "GameFramework/Actor.h"

namespace
{
	FString FormatDebugOverlayTargetSource(EDebugOverlayTargetSource InSource)
	{
		switch (InSource)
		{
		case EDebugOverlayTargetSource::Trace:
			return TEXT("TargetComponent.Trace");
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
}
