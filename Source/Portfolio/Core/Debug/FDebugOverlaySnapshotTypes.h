#pragma once

#include "CoreMinimal.h"

class AActor;

// ===== Capture State =====

enum class EDebugOverlayCaptureState : uint8
{
	NotCaptured,
	Captured,
	Unavailable,
	Stale,
};

// ===== Event Log Entry =====

struct PORTFOLIO_API FDebugOverlayEventEntry
{
	uint64 FrameNumber = 0;
	float WorldTimeSeconds = 0.f;
	FString Category;
	FString EventName;
	FString OwnerName;
	FString SourceName;
	FString TargetName;
	TWeakObjectPtr<AActor> OwnerActor;
	TWeakObjectPtr<AActor> SourceActor;
	TWeakObjectPtr<AActor> TargetActor;
	FString Summary;
};

// ===== Recent Summary Snapshots =====

struct PORTFOLIO_API FDebugOverlayExecutionSummary
{
	EDebugOverlayCaptureState CaptureState = EDebugOverlayCaptureState::NotCaptured;
	uint64 FrameNumber = 0;
	float WorldTimeSeconds = 0.f;
	FString OwnerName;
	FString Domain;
	FString Decision;
	FString ApplyMode;
	FString RejectReason;
	FString Summary;
};

struct PORTFOLIO_API FDebugOverlayCombatSummary
{
	EDebugOverlayCaptureState CaptureState = EDebugOverlayCaptureState::NotCaptured;
	uint64 FrameNumber = 0;
	float WorldTimeSeconds = 0.f;
	FString SourceName;
	FString TargetName;
	FString DamageCauserName;
	int32 HitWindowId = INDEX_NONE;
	FString HitWindowState;
	FString DefenseOutcome;
	FString ReactionOutcome;
	bool bHasDamageCommit = false;
	bool bDamageCommitted = false;
	bool bHasDamageBreakdown = false;
	float RequestDamage = 0.f;
	float MitigatedDamage = 0.f;
	float FinalTakenDamage = 0.f;
	float CommittedDamage = 0.f;
	FString Summary;
};

struct PORTFOLIO_API FDebugOverlayAISummary
{
	EDebugOverlayCaptureState CaptureState = EDebugOverlayCaptureState::NotCaptured;
	uint64 FrameNumber = 0;
	float WorldTimeSeconds = 0.f;
	FString ControllerName;
	FString PawnName;
	FString TargetName;
	FString IntentState;
	FString SubState;
	FString RequestResult;
	FString RejectReason;
	FString RuntimeLODTier;
	FString Summary;
};

// ===== Enemy Combat Target Facing =====

struct PORTFOLIO_API FDebugOverlayFacingSummary
{
	EDebugOverlayCaptureState CaptureState = EDebugOverlayCaptureState::NotCaptured;
	uint64 FrameNumber = 0;
	float WorldTimeSeconds = 0.f;
	uint32 TransitionSequence = 0;
	FString OwnerName;
	FString OwnerControllerName;
	FString BoundControllerName;
	FString CombatTargetName;
	FString GameplayFocusName;
	FString RotationMode;
	FString PolicyState;
	FString ExpectedFocusDirective;
	FString ExpectedRotationDirective;
	FString EventName;
	FString Decision;
	bool bControllerBindingMatchesOwner = false;
	FString Summary;
};

struct PORTFOLIO_API FDebugOverlayFacingTransition
{
	FDebugOverlayFacingSummary Current;
	TWeakObjectPtr<AActor> OwnerActor;
	TWeakObjectPtr<AActor> CombatTargetActor;
	FString PreviousGameplayFocusName;
	FString PreviousRotationMode;
};

// ===== Snapshot Aggregate =====

struct PORTFOLIO_API FDebugOverlaySnapshot
{
	FDebugOverlayExecutionSummary LastExecution;
	FDebugOverlayCombatSummary LastCombat;
	FDebugOverlayAISummary LastAI;
	TMap<FString, FDebugOverlayAISummary> LastAIByPawnName;
	TMap<FString, FDebugOverlayFacingSummary> LastFacingByPawnName;
	TArray<FDebugOverlayEventEntry> RecentEvents;
};
