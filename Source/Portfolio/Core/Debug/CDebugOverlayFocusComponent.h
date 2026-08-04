#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CDebugOverlayFocusComponent.generated.h"

enum class EDebugOverlayFocusSource : uint8
{
	None,
	NearestEnemy,
	RecentCombat,
	WorldScanFallback,
	GameplayTarget,
	EditorSelection,
};

enum class EDebugOverlayFocusCommandType : uint8
{
	None,
	SelectNearestTarget,
	SelectActorTarget,
	SelectRecentCombatTarget,
	ClearTarget,
};

enum class EDebugOverlayFocusCommandStatus : uint8
{
	None,
	Selected,
	Cleared,
	InvalidContext,
	NoEnemy,
	OutOfRange,
	NoActorName,
	NoActor,
	NotEnemy,
	NoRecentCombat,
};

struct PORTFOLIO_API FDebugOverlayFocusCommandResult
{
	EDebugOverlayFocusCommandType CommandType = EDebugOverlayFocusCommandType::None;
	EDebugOverlayFocusCommandStatus Status = EDebugOverlayFocusCommandStatus::None;
	EDebugOverlayFocusSource FocusMode = EDebugOverlayFocusSource::None;
	FString ActorName;
	FString ClassName;
	float Distance = 0.f;
	float Radius = 0.f;
	FString SummaryTextOverride;
};

UCLASS(ClassGroup = (Debug))
class PORTFOLIO_API UCDebugOverlayFocusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCDebugOverlayFocusComponent();

private:
	TWeakObjectPtr<AActor> DebugOverlayFocusActor;
	EDebugOverlayFocusSource DebugOverlayFocusSource = EDebugOverlayFocusSource::None;
	FDebugOverlayFocusCommandResult DebugOverlayFocusCommandResult;

public:
	// Focus Query
	bool HasDebugOverlayFocus() const;
	bool HasDebugOverlayFocusCommandResult() const;
	AActor* GetDebugOverlayFocusActor() const;
	FString GetDebugOverlayFocusActorText() const;
	FString GetDebugOverlayFocusModeText() const;
	const FDebugOverlayFocusCommandResult& GetDebugOverlayFocusCommandResult() const;
	FString GetDebugOverlayFocusCommandResultText() const;

	// Focus Mutation
	void SetDebugOverlayFocus(AActor* InFocusActor, EDebugOverlayFocusSource InSource);
	void ClearDebugOverlayFocus();
	void SetDebugOverlayFocusCommandResult(const FDebugOverlayFocusCommandResult& InResult);
	void ClearDebugOverlayFocusCommandResult();
};
