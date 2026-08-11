#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlaySnapshotTypes.h"
#include "Core/Debug/FTargetingDebug.h"

class ACEnemy;
class APawn;
class UObject;
class UWorld;

enum class EDebugOverlayRecentAIEventViewState : uint8
{
	NoTarget,
	NotCaptured,
	Stale,
	Captured,
};

enum class EDebugOverlayRecentExecutionViewState : uint8
{
	NotCaptured,
	NoActor,
	NoEvents,
	Captured,
};

struct FDebugOverlayRecentExecutionViewData
{
	EDebugOverlayRecentExecutionViewState State = EDebugOverlayRecentExecutionViewState::NotCaptured;
	FString HeaderText;
	FString SummaryText;
};

struct FDebugOverlayActorStatusViewData
{
	FString StateText;
	FString ActionText;
	FString ReactionText;
	FString HealthText;
	FString StaggerText;
	FString GuardText;
	FString MovementText;
	FString RuntimeLODText;
};

struct FDebugOverlayCurrentAIViewData
{
	bool bHasEnemy = false;
	FString ControllerText;
	FString PawnText;
	FString TargetText;
	FString IntentStateText;
	FString ReturnHomeText;
	FString UsePatrolText;
	FString HasLOSText;
	FString DistanceToTargetText;
	FString IsCombatActionText;
};

struct FDebugOverlayRecentAIEventViewData
{
	EDebugOverlayRecentAIEventViewState State = EDebugOverlayRecentAIEventViewState::NoTarget;
	FString TaskText;
	FString ResultText;
	FString AgeText;
	FString RejectReasonText;
	FString StaleAgeText;
};

struct FDebugOverlayFocusViewData
{
	FString CurrentSourceText;
	FString CurrentActorNameText;
	FString FocusDriverText;
	FString RecentFocusStateText;
};

struct FDebugOverlayPlayerTargetingViewData
{
	FTargetingDebugOverlayDetails Details;
};

struct FDebugOverlayActorPanelViewData
{
	FString HeaderText;
	bool bIncludeFocus = false;
	FDebugOverlayFocusViewData Focus;
	bool bIncludeTargeting = false;
	FDebugOverlayPlayerTargetingViewData Targeting;
	bool bAppendBlankBeforeStatus = false;
	FDebugOverlayActorStatusViewData Status;
	FDebugOverlayRecentExecutionViewData RecentExecution;
	bool bIncludeCurrentAI = false;
	FDebugOverlayCurrentAIViewData CurrentAI;
	bool bIncludeRecentAIEvent = false;
	FDebugOverlayRecentAIEventViewData RecentAIEvent;
};

struct FDebugOverlayEventLogEntryViewData
{
	FString CategoryText;
	FString EventNameText;
	FString SummaryText;
};

struct FDebugOverlayEventLogViewData
{
	bool bHasSnapshot = false;
	int32 DisplayLimit = 0;
	FString FilterText;
	TArray<FDebugOverlayEventLogEntryViewData> Entries;
};

struct FDebugOverlayRecentSummaryBlockViewData
{
	FString HeaderText;
	FString SummaryText;
	EDebugOverlayCaptureState CaptureState = EDebugOverlayCaptureState::NotCaptured;
	bool bHasSnapshot = false;
	bool bAppendLeadingBlank = false;
};

struct FDebugOverlayWorldSummaryViewData
{
	FString HeaderText;
	TArray<FDebugOverlayRecentSummaryBlockViewData> SummaryBlocks;
};

struct FDebugOverlayViewData
{
	FString MainPanelTitle;
	TArray<FDebugOverlayActorPanelViewData> ActorPanels;
	FString EventLogPanelTitle;
	FDebugOverlayEventLogViewData EventLog;
	FString WorldSummaryPanelTitle;
	FDebugOverlayWorldSummaryViewData WorldSummary;
};
