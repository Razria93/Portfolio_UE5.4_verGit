#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Debug/FDebugOverlayFocusTypes.h"
#include "CDebugOverlayFocusComponent.generated.h"

UCLASS(ClassGroup = (Debug))
class PORTFOLIO_API UCDebugOverlayFocusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCDebugOverlayFocusComponent();

private:
	TWeakObjectPtr<AActor> DebugOverlayFocusActor;
	EDebugOverlayFocusSource DebugOverlayFocusSource = EDebugOverlayFocusSource::None;
	EDebugOverlayFocusDriver DebugOverlayFocusDriver = EDebugOverlayFocusDriver::None;
	EDebugOverlayRecentFocusState DebugOverlayRecentFocusState = EDebugOverlayRecentFocusState::None;

public:
	// Focus Value Query
	bool HasDebugOverlayFocus() const;
	AActor* GetDebugOverlayFocusActor() const;
	EDebugOverlayFocusSource GetDebugOverlayFocusSource() const;
	EDebugOverlayFocusDriver GetDebugOverlayFocusDriver() const;
	EDebugOverlayRecentFocusState GetDebugOverlayRecentFocusState() const;

	// Focus Text Query
	FString GetDebugOverlayFocusActorText() const;
	FString GetDebugOverlayFocusActorNameText() const;
	FString GetDebugOverlayFocusSourceText() const;
	FString GetDebugOverlayFocusDriverText() const;
	FString GetDebugOverlayRecentFocusStateText() const;

	// Focus Mutation
	// Set
	void SetDebugOverlayFocusActorAndSource(AActor* InFocusActor, EDebugOverlayFocusSource InSource);
	void SetDebugOverlayFocusDriver(EDebugOverlayFocusDriver InDriver);
	void SetDebugOverlayRecentFocusState(EDebugOverlayRecentFocusState InState);

	// Clear
	void ClearDebugOverlayFocusActorAndSource();
	void ClearDebugOverlayFocusDriver();
	void ClearDebugOverlayRecentFocusState();
};
