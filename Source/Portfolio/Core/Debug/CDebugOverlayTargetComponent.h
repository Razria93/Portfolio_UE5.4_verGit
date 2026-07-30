#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CDebugOverlayTargetComponent.generated.h"

enum class EDebugOverlayTargetSource : uint8
{
	None,
	Trace,
	Nearest,
};

UCLASS(ClassGroup = (Debug))
class PORTFOLIO_API UCDebugOverlayTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCDebugOverlayTargetComponent();

private:
	TWeakObjectPtr<AActor> DebugOverlayTargetActor;
	EDebugOverlayTargetSource DebugOverlayTargetSource = EDebugOverlayTargetSource::None;
	FString LastDebugOverlayTraceSummary;

public:
	// Query
	bool HasDebugOverlayTarget() const;
	AActor* GetDebugOverlayTargetActor() const;
	FString GetDebugOverlayTargetSummary() const;
	FString GetDebugOverlayTargetSource() const;
	FString GetDebugOverlayTraceSummary() const;

	// Mutation
	void SetDebugOverlayTarget(AActor* InTargetActor, EDebugOverlayTargetSource InSource);
	void ClearDebugOverlayTarget();
	void RecordDebugOverlayTraceSummary(const FString& InSummary);
};
