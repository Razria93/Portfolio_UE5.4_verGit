#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CDebugOverlayTargetComponent.generated.h"

enum class EDebugOverlayTargetSource : uint8
{
	None,
	Nearest,
	EditorSelection,
};

UCLASS(ClassGroup = (Debug))
class PORTFOLIO_API UCDebugOverlayTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCDebugOverlayTargetComponent();

private:
	TWeakObjectPtr<AActor> DebugOverlayFocusActor;
	EDebugOverlayTargetSource DebugOverlayFocusSource = EDebugOverlayTargetSource::None;
	FString DebugOverlayFocusCommandResult;

public:
	// Focus Query
	bool HasDebugOverlayFocus() const;
	bool HasDebugOverlayFocusCommandResult() const;
	AActor* GetDebugOverlayFocusActor() const;
	FString GetDebugOverlayFocusActorText() const;
	FString GetDebugOverlayFocusModeText() const;
	FString GetDebugOverlayFocusCommandResultText() const;

	// Focus Mutation
	void SetDebugOverlayFocus(AActor* InFocusActor, EDebugOverlayTargetSource InSource);
	void ClearDebugOverlayFocus();
	void SetDebugOverlayFocusCommandResult(const FString& InResultText);
	void ClearDebugOverlayFocusCommandResult();

	// Compatibility Query
	bool HasDebugOverlayTarget() const;
	bool HasDebugOverlaySelectionSummary() const;
	AActor* GetDebugOverlayTargetActor() const;
	FString GetDebugOverlayTargetSummary() const;
	FString GetDebugOverlayTargetSource() const;
	FString GetDebugOverlaySelectionSummary() const;

	// Compatibility Mutation
	void SetDebugOverlayTarget(AActor* InTargetActor, EDebugOverlayTargetSource InSource);
	void ClearDebugOverlayTarget();
	void SetDebugOverlaySelectionSummary(const FString& InSummary);
	void ClearDebugOverlaySelectionSummary();
};
