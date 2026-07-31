#pragma once

#include "CoreMinimal.h"

enum class EDebugOverlayCaptureState : uint8
{
	NotCaptured,
	Captured,
	Unavailable,
	Stale,
};

struct PORTFOLIO_API FDebugOverlayEventEntry
{
	uint64 FrameNumber = 0;
	float WorldTimeSeconds = 0.f;
	FString Category;
	FString EventName;
	FString OwnerName;
	FString SourceName;
	FString TargetName;
	FString Summary;
};

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
	bool bHasDamageCommit = false;
	bool bDamageCommitted = false;
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

struct PORTFOLIO_API FDebugOverlaySnapshot
{
	FDebugOverlayExecutionSummary LastExecution;
	FDebugOverlayCombatSummary LastCombat;
	FDebugOverlayAISummary LastAI;
	TArray<FDebugOverlayEventEntry> RecentEvents;
};
