#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CDebugOverlayFocusComponent.generated.h"

enum class EDebugOverlayFocusSource : uint8
{
	None,
	Nearest,
	EditorSelection,
};

using EDebugOverlayTargetSource = EDebugOverlayFocusSource;

UCLASS(ClassGroup = (Debug))
class PORTFOLIO_API UCDebugOverlayFocusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCDebugOverlayFocusComponent();

private:
	TWeakObjectPtr<AActor> DebugOverlayFocusActor;
	EDebugOverlayFocusSource DebugOverlayFocusSource = EDebugOverlayFocusSource::None;
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
	void SetDebugOverlayFocus(AActor* InFocusActor, EDebugOverlayFocusSource InSource);
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
	void SetDebugOverlayTarget(AActor* InTargetActor, EDebugOverlayFocusSource InSource);
	void ClearDebugOverlayTarget();
	void SetDebugOverlaySelectionSummary(const FString& InSummary);
	void ClearDebugOverlaySelectionSummary();
};
