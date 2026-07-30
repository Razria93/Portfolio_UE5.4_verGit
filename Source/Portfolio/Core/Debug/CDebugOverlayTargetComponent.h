#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CDebugOverlayTargetComponent.generated.h"

UCLASS(ClassGroup = (Debug))
class PORTFOLIO_API UCDebugOverlayTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCDebugOverlayTargetComponent();

private:
	TWeakObjectPtr<AActor> DebugOverlayTargetActor;

public:
	// Query
	bool HasDebugOverlayTarget() const;
	AActor* GetDebugOverlayTargetActor() const;
	FString GetDebugOverlayTargetSummary() const;
	FString GetDebugOverlayTargetSource() const;

	// Mutation
	void SetDebugOverlayTarget(AActor* InTargetActor);
	void ClearDebugOverlayTarget();
};
