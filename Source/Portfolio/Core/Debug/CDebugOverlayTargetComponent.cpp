#include "Core/Debug/CDebugOverlayTargetComponent.h"

#include "GameFramework/Actor.h"

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
	return TEXT("TargetComponent");
}

// Mutation
void UCDebugOverlayTargetComponent::SetDebugOverlayTarget(AActor* InTargetActor)
{
	DebugOverlayTargetActor = InTargetActor;
}

void UCDebugOverlayTargetComponent::ClearDebugOverlayTarget()
{
	DebugOverlayTargetActor.Reset();
}
